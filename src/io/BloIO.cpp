#include "io/BloIO.hpp"
#include <glad/glad.h>
#include "imgui.h"
#include <format>
#include "GenUtil.hpp"
#include "IconsForkAwesome.h"

namespace Blo {

void Resource::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    mType = static_cast<ResourceType>(stream->readUInt8());
    uint8_t len = stream->readUInt8();
    mPath = stream->readString(len);
    std::cout << "Resource Path: " << mPath << std::endl;
}

void Image::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    Resource::Load(stream, timg);
    std::shared_ptr<Archive::File> imgFile = timg->GetFile(mPath);

    if(imgFile != nullptr){
        std::cout << "Loading Image " << mPath << std::endl;
        bStream::CMemoryStream img(imgFile->GetData(), imgFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
        mTexture.Load(&img);

		glCreateTextures(GL_TEXTURE_2D, 1, &mTextureID);
		glTextureStorage2D(mTextureID, 1, GL_RGBA8, mTexture.mWidth, mTexture.mHeight);
		glTextureSubImage2D(mTextureID, 0, 0, 0, mTexture.mWidth, mTexture.mHeight, GL_RGBA, GL_UNSIGNED_BYTE, mTexture.GetData());
    }
}

Image::~Image(){
    glDeleteTextures(1, &mTextureID);
}

void Palette::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    Resource::Load(stream, timg);
}

bool Screen::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
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
        uint32_t paneType = stream->readUInt32();
        uint32_t paneLen = stream->readUInt32();
        
        switch (paneType) {
        case 0x50414E31: // PAN1
            std::cout << "Loading Pane" << std::endl;
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
    mParent = parent;
    parent->mChildren.push_back(shared_from_this());

    int numParams = stream->readUInt8() - 6;
    std::cout << "Loading pane " << (int)numParams << " params..." << std::endl;
    mVisible = stream->readUInt8() != 0;
    stream->skip(2);

    mID = stream->readUInt32();

    mRect[0] = stream->readUInt16();
    mRect[1] = stream->readUInt16();
    mRect[2] = stream->readUInt16();
    mRect[3] = stream->readUInt16();

    char temp[4];
    memcpy(temp, &mID, 4);
    std::cout << "Pane ID " << temp << " params..." << std::endl;

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
    
    std::cout << "Finished Loading Pane at " << std::hex << stream->tell() << std::dec << std::endl;

    return true;
}

bool Picture::Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    Pane::Load(stream, parent, timg);

    uint8_t numParams = stream->readUInt8() - 3;
    std::cout << "Loading picture " << (int)numParams << " params..." << std::endl;
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

void Window::Draw(){

    if(mContentTexture.mTextureID != 0xFFFFFFFF){
        ImGui::SetCursorPosX(mContentRect[0]);
        ImGui::SetCursorPosY(mContentRect[1]);

        ImGui::Image(static_cast<uintptr_t>(mContentTexture.mTextureID), {mContentTexture.mTexture.mWidth, mContentTexture.mTexture.mHeight}, {0.0, 0.0}, {1.0, 1.0});
    } else {
        auto WindowPos = ImGui::GetWindowPos();
        ImVec2 min = {WindowPos.x + mRect[0], WindowPos.y + mRect[1]};
        ImVec2 max = { mRect[2] + min.x, mRect[3] + min.y};
        ImGui::GetWindowDrawList()->AddRect(min, max, ImColor(0xFFFFFFFF), 0.0, ImDrawFlags_None, 2.0f);
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
        
        ImGui::Image(static_cast<uintptr_t>(mTexutres[0].mTextureID), {mTexutres[0].mTexture.mWidth , mTexutres[0].mTexture.mHeight}, UV0, UV1);
    }
}

void Pane::Draw(){
    ImGui::SetCursorPosX(mRect[0]);
    ImGui::SetCursorPosY(mRect[1]);
    ImGui::BeginChild(std::format("##pane{}", mID).c_str(), ImVec2(mRect[2], mRect[3]), ImGuiChildFlags_Border);

    for(auto child : mChildren){
        child->Draw();
    }

    ImGui::EndChild();
}

void Screen::Draw(){
    ImGui::BeginChild(std::format("##screen{}", mID).c_str(), ImVec2(mRect[2], mRect[3]), ImGuiChildFlags_Border);

    for(auto child : mChildren){
        child->Draw();
    }

    ImGui::EndChild();
}

void Screen::DrawHierarchy(){
    ImGui::BeginChild("##bloViewHierarchy");
    ImGui::Text("Screen Elements");
    ImGui::Indent();
    for(auto child : mChildren){
        child->DrawHierarchy();
    }
    ImGui::Unindent();
    ImGui::EndChild();
}

void Pane::DrawHierarchy(){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    if(ImGui::TreeNode("%4s", drawID)){
        for(auto child : mChildren){
            child->DrawHierarchy();
        }
        
        ImGui::TreePop();
    }
}

void Picture::DrawHierarchy(){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    if(ImGui::TreeNode("%4s", drawID)){
        ImGui::Text(std::format("Texture: {}", mTexutres[0].mPath).c_str());
        for(auto child : mChildren){
            child->DrawHierarchy();
        }
        
        ImGui::TreePop();
    }
}

void Window::DrawHierarchy(){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    if(ImGui::TreeNode("Window: %4s", drawID)){
        for(auto child : mChildren){
            child->DrawHierarchy();
        }
        
        ImGui::TreePop();
    }
}

}