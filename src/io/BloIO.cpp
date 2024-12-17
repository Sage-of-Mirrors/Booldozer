#include "io/BloIO.hpp"
#include <glad/glad.h>
#include "imgui.h"
#include <format>
#include "GenUtil.hpp"
#include "IconsForkAwesome.h"
#include "imgui_internal.h"

namespace Blo {

void Resource::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    mType = static_cast<ResourceType>(stream->readUInt8());
    uint8_t len = stream->readUInt8();
    mPath = stream->readString(len);
}

void Image::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    Resource::Load(stream, timg);
    std::shared_ptr<Archive::File> imgFile = timg->GetFile(mPath);

    if(imgFile != nullptr){
        //std::cout << "Loading Image " << mPath << std::endl;
        bStream::CMemoryStream img(imgFile->GetData(), imgFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
        mTexture.Load(&img);
        
		glCreateTextures(GL_TEXTURE_2D, 1, &mTextureID);
        glTextureParameteri(mTextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(mTextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(mTextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(mTextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureStorage2D(mTextureID, 1, GL_RGBA8, mTexture.mWidth, mTexture.mHeight);
		glTextureSubImage2D(mTextureID, 0, 0, 0, mTexture.mWidth, mTexture.mHeight, GL_RGBA, GL_UNSIGNED_BYTE, mTexture.GetData());
    } else {
        mTextureID = 0xFFFFFFFF;
        LGenUtility::Log << "Couldn't load image resource " << mPath << std::endl;
    }
}

Image::~Image(){
    glDeleteTextures(1, &mTextureID);
}

void Palette::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    Resource::Load(stream, timg);
}

bool Screen::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    mType = ElementType::Screen;
    if(stream->readUInt32() != 0x5343524e){ // 'SCRN'
        return false;
    }

    if(stream->readUInt32() != 0x626c6f31){ // 'SCRN'
        return false;
    }

    stream->skip(24);
    
    if(stream->readUInt32() != 0x494e4631){ // 'SCRN'
        return false;
    }

    std::size_t size = stream->readUInt32();
    mRect[0] = 0;
    mRect[1] = 0;
    mRect[2] = stream->readInt16();
    mRect[3] = stream->readInt16();
    mColor = stream->readUInt32();

    if(!LoadBlo1(stream, shared_from_this(), timg)) {
        return false;
    }

    return true;
}

bool Screen::LoadBlo1(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    std::shared_ptr<Pane> prev = parent;
    while(true){
        std::size_t paneStart = stream->tell();
        
        if(stream->tell() >= stream->getSize()) return true;

        uint32_t paneType = stream->readUInt32();
        uint32_t paneLen = stream->readUInt32();
        
        switch (paneType) {
        case 0x50414E31: // PAN1
            //std::cout << "Loading Pane" << std::endl;
            prev = std::make_shared<Pane>();
            prev->Load(stream, parent, timg);
            if(stream->tell() != paneStart + paneLen){
                stream->seek(paneStart + paneLen);
                break;
            }
            break;
		case 0x50494331: // PIC1
            prev = std::make_shared<Picture>();
            prev->Load(stream, parent, timg);
            if(stream->tell() != paneStart + paneLen){
                stream->seek(paneStart + paneLen);
                break;
            }
            break;
		case 0x42474E31: // BGN1
            stream->seek(paneStart + paneLen);
            if(!LoadBlo1(stream, prev, timg)){
                return false;
            }
            break;
		case 0x57494E31: // WIN1
            prev = std::make_shared<Window>();
            prev->Load(stream, parent, timg);
            if(stream->tell() != paneStart + paneLen){
                stream->seek(paneStart + paneLen);
                break;
            }
            break;
		case 0x54425831: // TBX1
            break;
		case 0x454E4431: // END1
            stream->seek(paneStart + paneLen);
            return true;
		case 0x45585431: // EXT1
            break;
        default:
            return false;
        }
    }
}

bool Pane::Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    mType = ElementType::Pane;
    mParent = parent;
    parent->mChildren.push_back(shared_from_this());

    int numParams = stream->readUInt8() - 6;
    //std::cout << "Loading pane " << (int)numParams << " params..." << std::endl;
    mVisible = stream->readUInt8() != 0;
    stream->skip(2);

    mID = stream->readUInt32();

    mRect[0] = stream->readUInt16();
    mRect[1] = stream->readUInt16();
    mRect[2] = stream->readUInt16();
    mRect[3] = stream->readUInt16();

    char temp[4];
    memcpy(temp, &mID, 4);
    //std::cout << "Pane ID " << temp << " params..." << std::endl;

    if(numParams > 0){
        mAngle = stream->readUInt16();
        numParams--;
    } else {
        mAngle = 0;
    }

    if(numParams > 0){
        mAnchor = static_cast<Anchor>(stream->readUInt8());
        numParams--;
    } else {
        mAnchor = Anchor::TopLeft; 
    }

    if(numParams > 0){
        mAplha = stream->readUInt8();
        numParams--;
    } else {
        mAplha = 0xFF;
    }

    if(numParams > 0){
        mInheritAlpha = stream->readUInt8() != 0;
        numParams--;
    } else {
        mInheritAlpha = true;
    }

    //stream->skip(4);
    
    //std::cout << "Finished Loading Pane at " << std::hex << stream->tell() << std::dec << std::endl;

    return true;
}

bool Picture::Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    Pane::Load(stream, parent, timg);
    mType = ElementType::Picture;

    uint8_t numParams = stream->readUInt8() - 3;
    //std::cout << "Loading picture " << (int)numParams << " params..." << std::endl;
    mTexutres[0].Load(stream, timg);
    mPalette.Load(stream, timg);
    mBinding = static_cast<Binding>(stream->readUInt8());

    if(numParams > 0){
        uint8_t bits = stream->readUInt8();
        mMirror = static_cast<MirrorMode>(bits & 3);
        mRotate = ((bits & 4) != 0);
        numParams--;
    } else {
        mMirror = MirrorMode::None;
        mRotate = false;
    }

    if(numParams > 0){
        uint8_t bits = stream->readUInt8();
        mWrapX = static_cast<WrapMode>((bits >> 2) & 3);
        mWrapY = static_cast<WrapMode>((bits >> 0) & 3);
        numParams--;
    } else {
        mWrapX = WrapMode::None;
        mWrapY = WrapMode::None;
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mFromColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
        numParams--;
    } else {
        mFromColor = {0.0f, 0.0f, 0.0f, 0.0f};
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mToColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
        numParams--;
    } else {
        mToColor = {1.0f, 1.0f, 1.0f, 1.0f};
    }

    for(int c = 0; c < 4; c++){
        if(numParams > 0){
            uint32_t color = stream->readUInt32();
            mColors[c] = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
            numParams--;
        } else {
            mColors[c] = {1.0f, 1.0f, 1.0f, 1.0f};
        }
    }

    stream->skip(4);
    return true;
}

bool Window::Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    Pane::Load(stream, parent, timg);
    mType = ElementType::Window;

    uint8_t numParams = stream->readUInt8() - 14;


    mContentRect[0] = stream->readUInt16();
    mContentRect[1] = stream->readUInt16();
    mContentRect[2] = stream->readUInt16();
    mContentRect[3] = stream->readUInt16();

    for(std::size_t i = 0; i < 4; i++){
        mTextures[i].Load(stream, timg);
    }

    mPalette.Load(stream, timg);

    uint8_t bits = stream->readUInt8();
    for(std::size_t i = 0; i < 4; i++){
        mTextures[i].mMirror = ((bits >> (6 - (i * 2))) & 3);
    }

    for(std::size_t i = 0; i < 4; i++){
        uint32_t color = stream->readUInt32();
        mTextures[i].mColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
    }

    if(numParams > 0){
        std::cout << "Loading Context Texture at " << stream->tell() << std::endl;
        mContentTexture.Load(stream, timg);
        numParams--;
    } else {
        mContentTexture.mTextureID = 0xFFFFFFFF;
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mFromColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
        numParams--;
    } else {
        mFromColor = {0.0f, 0.0f, 0.0f, 0.0f};
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mToColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
        numParams--;
    } else {
        mToColor = {1.0f, 1.0f, 1.0f, 1.0f};
    }

    stream->skip(4);

    return true;
}

bool Textbox::Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    Pane::Load(stream, parent, timg);
    mType = ElementType::Textbox;

    uint8_t numParams = stream->readUInt8() - 10;

    mFont.Load(stream, timg);

    uint32_t color = stream->readUInt32();
    mTopColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
    
    color = stream->readUInt32();
    mBottomColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};

    uint8_t alignment = stream->readUInt8();
    mHAlign = (alignment >> 2 & 3);
    mVAlign = (alignment & 3);

    mFontSpacing = stream->readUInt16();
    mFontLeading = stream->readUInt16();
    mFontWidth = stream->readUInt16();
    mFontHeight = stream->readUInt16();

    uint16_t len = stream->readUInt16();
    mText = stream->readString(len);

    if(numParams > 0){
        if(stream->readUInt8() != 0){
            mConnectParent = true;
        }
        numParams--;
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mFromColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
        numParams--;
    } else {
        mFromColor = {0.0f, 0.0f, 0.0f, 0.0f};
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mToColor = {((color >> 24) & 0xFF) / 0xFF, ((color >> 16) & 0xFF) / 0xFF, ((color >> 8) & 0xFF) / 0xFF, (color & 0xFF) / 0xFF};
        numParams--;
    } else {
        mToColor = {1.0f, 1.0f, 1.0f, 1.0f};
    }

    stream->skip(4);

    return true;
}

void Window::Draw(){
    // draw borders

    if(mTextures[0].mTextureID != 0xFFFFFFFF && mTextures[1].mTextureID != 0xFFFFFFFF && mTextures[2].mTextureID != 0xFFFFFFFF && mTextures[3].mTextureID != 0xFFFFFFFF){
        ImGui::SetCursorPosX(mRect[0]);
        ImGui::SetCursorPosY(mRect[1]);
        ImGui::Image(static_cast<uintptr_t>(mTextures[0].mTextureID), {static_cast<float>(mTextures[0].mTexture.mWidth), static_cast<float>(mTextures[0].mTexture.mHeight)}, {0.0, 0.0}, {1.0, 1.0}, ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha));
        
        ImGui::SetCursorPosX((mRect[0] + mRect[2]) - mTextures[1].mTexture.mWidth);
        ImGui::SetCursorPosY(mRect[1]);
        ImGui::Image(static_cast<uintptr_t>(mTextures[1].mTextureID), {static_cast<float>(mTextures[1].mTexture.mWidth), static_cast<float>(mTextures[1].mTexture.mHeight)}, {1.0, 0.0}, {0.0, 1.0}, ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha));
        
        ImGui::SetCursorPosX(mRect[0]);
        ImGui::SetCursorPosY((mRect[1] + mRect[3]) - mTextures[3].mTexture.mHeight);
        ImGui::Image(static_cast<uintptr_t>(mTextures[2].mTextureID), {static_cast<float>(mTextures[2].mTexture.mWidth), static_cast<float>(mTextures[2].mTexture.mHeight)}, {0.0, 1.0}, {1.0, 0.0}, ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha));
        
        ImGui::SetCursorPosX((mRect[0] + mRect[2]) - mTextures[3].mTexture.mWidth);
        ImGui::SetCursorPosY((mRect[1] + mRect[3]) - mTextures[3].mTexture.mHeight);
        ImGui::Image(static_cast<uintptr_t>(mTextures[3].mTextureID), {static_cast<float>(mTextures[3].mTexture.mWidth), static_cast<float>(mTextures[3].mTexture.mHeight)}, {1.0, 1.0}, {0.0, 0.0}, ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha));

        // edges
        ImGui::SetCursorPosY(mRect[1]);
        ImGui::SetCursorPosX(mRect[0] + mTextures[1].mTexture.mWidth);
        ImGui::Image(static_cast<uintptr_t>(mTextures[1].mTextureID), {static_cast<float>(mRect[2] - (mTextures[1].mTexture.mWidth*2)), static_cast<float>(mTextures[1].mTexture.mHeight)}, {0.9999, 0.0}, {1.0, 1.0}, ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha));

        ImGui::SetCursorPosY(mRect[1] + mTextures[1].mTexture.mHeight);
        ImGui::SetCursorPosX(mRect[0]);
        ImGui::Image(static_cast<uintptr_t>(mTextures[1].mTextureID), {static_cast<float>(mTextures[2].mTexture.mWidth), static_cast<float>(mRect[3] - (mTextures[2].mTexture.mHeight * 2))}, {0.0, 0.9999}, {1.0, 1.0}, ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha));

        ImGui::SetCursorPosY(mRect[1] + mTextures[2].mTexture.mHeight);
        ImGui::SetCursorPosX((mRect[0] + mRect[2]) - mTextures[2].mTexture.mWidth);
        ImGui::Image(static_cast<uintptr_t>(mTextures[2].mTextureID), {static_cast<float>(mTextures[2].mTexture.mWidth), static_cast<float>(mRect[3] - (mTextures[2].mTexture.mHeight * 2))}, {1.0, 0.9999}, {0.0, 1.0}, ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha));

        ImGui::SetCursorPosY(mRect[1] + mRect[3] - mTextures[1].mTexture.mHeight);
        ImGui::SetCursorPosX(mRect[0] + mTextures[1].mTexture.mWidth);
        ImGui::Image(static_cast<uintptr_t>(mTextures[1].mTextureID), {static_cast<float>(mRect[2] - (mTextures[1].mTexture.mWidth*2)), static_cast<float>(mTextures[1].mTexture.mHeight)}, {0.9999, 1.0}, {1.0, 0.0}, ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha));

    }

    auto WindowPos = ImGui::GetWindowPos();
    ImVec2 min = {WindowPos.x + mRect[0] + mContentRect[0], WindowPos.y + mRect[1] + mContentRect[1]};
    ImVec2 max = { mContentRect[2] + min.x, mContentRect[3] + min.y};

    // draw contents
    if(mContentTexture.mTextureID != 0xFFFFFFFF){
        ImGui::SetCursorPosX(mContentRect[0]);
        ImGui::SetCursorPosY(mContentRect[1]);

        float hf = mContentRect[2] / mContentTexture.mTexture.mWidth;
        float vf = mContentRect[3] / mContentTexture.mTexture.mHeight; 

        ImGui::Image(static_cast<uintptr_t>(mContentTexture.mTextureID), {static_cast<float>(mContentRect[2]), static_cast<float>(mContentRect[3])}, {0.0, 0.0}, {hf, vf});
    } else {
        ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mAplha)));
    }
}

void Picture::Draw(){
    ImGui::SetCursorPosX(mRect[0]);
    ImGui::SetCursorPosY(mRect[1]);
    
    if(mVisible){
        ImVec2 UV0 (0.0f, 0.0f);
        ImVec2 UV1 (1.0f, 1.0f);
        
        if(mMirror & MirrorMode::X){
            UV0.x = 1.0f;
            UV1.x = 0.0f;
        }

        if(mMirror & MirrorMode::Y){
            UV0.y = 1.0f;
            UV1.y = 0.0f;
        }
        
        int vidx = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        ImGui::GetWindowDrawList()->AddImageQuad(
            static_cast<uintptr_t>(mTexutres[0].mTextureID),
            {ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y},
            {ImGui::GetCursorScreenPos().x + mTexutres[0].mTexture.mWidth, ImGui::GetCursorScreenPos().y},
            {ImGui::GetCursorScreenPos().x + mTexutres[0].mTexture.mWidth, ImGui::GetCursorScreenPos().y + mTexutres[0].mTexture.mHeight},
            {ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + mTexutres[0].mTexture.mHeight},
            UV0,
            {UV1.x, UV0.y},
            UV1,
            {UV0.x, UV1.y},
            ImColor(mColors[1].r, mColors[1].g, mColors[1].b, mColors[1].a)
        );
        int evidx = ImGui::GetWindowDrawList()->VtxBuffer.Size;

        ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImGui::GetWindowDrawList(), vidx + 0, vidx + 2, {0.0, 0.0}, {0.0, 1.0}, ImColor(ImVec4(mFromColor.r, mFromColor.g, mFromColor.b, mFromColor.a)), ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a)));
    }
}

void Pane::Draw(){
    ImGui::SetCursorPosX(mRect[0]);
    ImGui::SetCursorPosY(mRect[1]);
    ImGui::BeginChild(std::format("Pane##pane{}", mID).c_str(), ImVec2(mRect[2], mRect[3]), ImGuiChildFlags_Border);

    ImGui::PushID(std::format("##PreviewPane{}", mID).c_str());
    for(auto child: mChildren){
        child->Draw();
    }
    ImGui::PopID();

    ImGui::EndChild();
}

void Textbox::Draw(){
    ImGui::SetCursorPosX(mRect[0]);
    ImGui::SetCursorPosY(mRect[1]);

    int vidx = ImGui::GetWindowDrawList()->VtxBuffer.Size;
    ImGui::Text(mText.c_str());
    ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImGui::GetWindowDrawList(), vidx + 0, vidx + 2, {0.0, 0.0}, {0.0, 1.0}, ImColor(ImVec4(mTopColor.r, mTopColor.g, mTopColor.b, mTopColor.a)), ImColor(ImVec4(mBottomColor.r, mBottomColor.g, mBottomColor.b, mBottomColor.a)));

    ImGui::PushID(std::format("##PreviewPane{}", mID).c_str());
    for(auto child: mChildren){
        child->Draw();
    }
    ImGui::PopID();

    ImGui::EndChild();
}

void Screen::Draw(){
    ImGui::BeginChild(std::format("Screen##screen{}", mID).c_str(), ImVec2(mRect[2], mRect[3]), ImGuiChildFlags_Border);

    ImGui::PushID(std::format("##PreviewScreen{}", mID).c_str());
    for(auto child: mChildren){
        child->Draw();
    }
    ImGui::PopID();

    ImGui::EndChild();
}

void Screen::DrawHierarchy(std::shared_ptr<Blo::Pane> selection){
    ImGui::BeginChild("##bloViewHierarchy");
    ImGui::Text("Screen Elements");

    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags, "Screen %4s", drawID)){
        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }

        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}

void Pane::DrawHierarchy(std::shared_ptr<Blo::Pane> selection){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags, "%4s", drawID)){
        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }

        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        
        ImGui::TreePop();
    }
}

void Picture::DrawHierarchy(std::shared_ptr<Blo::Pane> selection){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags, "%4s", drawID)){
        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }

        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        
        ImGui::TreePop();
    }
}

void Window::DrawHierarchy(std::shared_ptr<Blo::Pane> selection){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags)){
        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }
        
        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        
        ImGui::TreePop();
    }
}

void Textbox::DrawHierarchy(std::shared_ptr<Blo::Pane> selection) {
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags)){
        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }
        
        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        
        ImGui::TreePop();
    }
}

}