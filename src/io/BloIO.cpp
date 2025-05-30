#include "io/BloIO.hpp"
#include <glad/glad.h>
#include "imgui.h"
#include <format>
#include "GenUtil.hpp"
#include "IconsForkAwesome.h"
#include "imgui_internal.h"

namespace Blo {

static int blockCount = 0;

Pane::Pane() : mType(ElementType::Pane), mID(0x50414E45) {}

Pane::Pane(ElementType t, uint32_t id) : mType(t), mID(id) {
    mAlpha = 0x00;
}

Screen::Screen() : Pane(ElementType::Screen, 0x5343524E) {}

Picture::Picture() : Pane(ElementType::Picture, 0x50494354) {
    mTextures[0] = std::make_shared<Image>();
    mBinding = (uint8_t)Binding::Top;
    mAnchor = (uint8_t)Anchor::CenterMiddle;
    mMirror = MirrorMode::None;
    mWrapX = 3;
    mWrapY = 3;

    mPictArgs["mirror"] = true;
    mPictArgs["wrap"] = true; 
    mPictArgs["fromColor"] = true;
    mPictArgs["toColor"] = true;

}

Textbox::Textbox() : Pane(ElementType::Textbox, 0x54585442) {
    mText = "Text";
    mTextboxArgs["connectParent"] = true;
    mTextboxArgs["fromColor"] = true;
    mTextboxArgs["toColor"] = true;
}

Window::Window() : Pane(ElementType::Window, 0x57494E44) {
    mTextures[0] = std::make_shared<Image>();
    mTextures[1] = std::make_shared<Image>(); 
    mTextures[1]->mMirror |= MirrorMode::X;
    mTextures[2] = std::make_shared<Image>(); 
    mTextures[2]->mMirror |= MirrorMode::Y; 
    mTextures[3] = std::make_shared<Image>();
    mTextures[3]->mMirror |= MirrorMode::X | MirrorMode::Y;
    mContentTexture = std::make_shared<Image>();
    mContentTexture->mTextureID = 0xFFFFFFFF;

    mWindowArgs["contenttex"] = true;
    mWindowArgs["fromColor"] = true;
    mWindowArgs["toColor"] = true;
}

void Resource::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    mType = stream->readUInt8();
    uint8_t len = stream->readUInt8();
    mPath = stream->readString(len);
}

void Resource::Save(bStream::CStream* stream){
    stream->writeUInt8(mType);
    stream->writeUInt8(mPath.size());
    if(mPath.size() != 0){
        stream->writeString(mPath);
    }
}

void Image::Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg){
    Resource::Load(stream, timg);
    std::shared_ptr<Archive::File> imgFile = timg->GetFile(mPath);

    if(imgFile == nullptr){
        std::string lowerPath = mPath;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), [](unsigned char c){ return std::tolower(c); });
        imgFile = timg->GetFile(lowerPath);
    }

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
    if(stream->readUInt32() != 0x5343524E){
        return false;
    }

    if(stream->readUInt32() != 0x626C6f31){
        return false;
    }

    stream->skip(24);
    
    if(stream->readUInt32() != 0x494E4631){
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

void SaveBlo1(bStream::CStream* stream, std::vector<std::shared_ptr<Pane>>& children){
    LGenUtility::Log << "Writing Children" << std::endl;
    blockCount++;
    stream->writeUInt32(0x42474E31); // BGN1
    stream->writeUInt32(8);

    for(std::size_t child = 0; child < children.size(); child++){
        uint32_t paneType = 0;
        std::size_t panePos = stream->tell();
        stream->writeUInt32(0);
        stream->writeUInt32(0);
        switch (children[child]->Type()){
        case ElementType::Pane:
            paneType = 0x50414E31;
            children[child]->Save(stream);
            break;
        case ElementType::Picture:
            paneType = 0x50494331;
            std::reinterpret_pointer_cast<Picture>(children[child])->Save(stream);
            break;
        case ElementType::Window:
            paneType = 0x57494E31;
            std::reinterpret_pointer_cast<Window>(children[child])->Save(stream);
            break;
        case ElementType::Textbox:
            paneType = 0x54425831;
            std::reinterpret_pointer_cast<Textbox>(children[child])->Save(stream);
            break;
        }
        std::size_t listPos = stream->tell();
        stream->seek(panePos);
        stream->writeUInt32(paneType);
        stream->writeUInt32(listPos - panePos);
        stream->seek(listPos);

        if(children[child]->mChildren.size() > 0){
            SaveBlo1(stream, children[child]->mChildren);
        }

    }
    stream->writeUInt32(0x454E4431); // END1
    blockCount++;
    stream->writeUInt32(8);
    LGenUtility::Log << "Finished writing children" << std::endl;
}

void Screen::Save(bStream::CStream* stream){
    blockCount = 0;
    LGenUtility::Log << "Writing Screen" << std::endl;
    stream->writeUInt32(0x5343524E);
    stream->writeUInt32(0x626C6f31);

    stream->writeUInt32(0);
    stream->writeUInt32(0);
    for(;stream->tell() < 0x20;) stream->writeUInt8(0);

    stream->writeUInt32(0x494E4631);
    blockCount++;
    stream->writeUInt32(0x10);

    stream->writeInt16(mRect[2]);
    stream->writeInt16(mRect[3]);
    stream->writeUInt32(mColor);
    
    LGenUtility::Log << "Writing Screen Children" << std::endl;
    for(std::size_t child = 0; child < mChildren.size(); child++){
        uint32_t paneType = 0;
        std::size_t panePos = stream->tell();
        stream->writeUInt32(0);
        stream->writeUInt32(0);
        switch (mChildren[child]->Type()){
        case ElementType::Pane:
            paneType = 0x50414E31;
            mChildren[child]->Save(stream);
            break;
        case ElementType::Picture:
            paneType = 0x50494331;
            std::reinterpret_pointer_cast<Picture>(mChildren[child])->Save(stream);
            break;
        case ElementType::Window:
            paneType = 0x57494E31;
            std::reinterpret_pointer_cast<Window>(mChildren[child])->Save(stream);
            break;
        case ElementType::Textbox:
            paneType = 0x54425831;
            std::reinterpret_pointer_cast<Textbox>(mChildren[child])->Save(stream);
            break;
        }
        std::size_t listPos = stream->tell();
        stream->seek(panePos);
        stream->writeUInt32(paneType);
        stream->writeUInt32(listPos - panePos);
        stream->seek(listPos);
        
        if(mChildren[child]->mChildren.size() > 0){
            SaveBlo1(stream, mChildren[child]->mChildren);
        }
    
    }
    LGenUtility::Log << "Finished Eriting Screen Children" << std::endl;

    stream->writeUInt32(0x45585431);
    blockCount++;
    stream->writeUInt32(8);
    for(int x = 0; x < Util::AlignTo(stream->tell(), 32) - stream->tell(); x++){
        stream->writeUInt8(0);
    }
    std::size_t endPos = stream->tell();
    stream->seek(0x08);
    stream->writeUInt32(endPos);
    stream->writeUInt32(blockCount);
}

void Pane::Save(bStream::CStream* stream){
    blockCount++;
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)+1] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    //LGenUtility::Log << "ID is " << drawID << " arg count " << (6 + mPaneArgs["angle"] + mPaneArgs["anchor"] + mPaneArgs["alpha"] + mPaneArgs["inheritAlpha"]) << " writing at " << std::hex << stream->tell() << std::dec << std::endl;
    stream->writeUInt8(6 + mPaneArgs["angle"] + mPaneArgs["anchor"] + mPaneArgs["alpha"] + mPaneArgs["inheritAlpha"]);
    stream->writeUInt8(mVisible);
    stream->writeUInt16(0);
    stream->writeUInt32(mID);
    stream->writeInt16(mRect[0]);
    stream->writeInt16(mRect[1]);
    stream->writeInt16(mRect[2]);
    stream->writeInt16(mRect[3]);

    if(mPaneArgs["angle"]){
        stream->writeUInt16(mAngle);
    }

    if(mPaneArgs["anchor"]){
        stream->writeUInt8((uint8_t)mAnchor);
    }

    if(mPaneArgs["alpha"]){
        stream->writeUInt8(mAlpha);
    }

    if(mPaneArgs["inheritAlpha"]){
        stream->writeUInt8(mInheritAlpha);
    }

    //stream->writeUInt32(0);
}

void Picture::Save(bStream::CStream* stream){
    LGenUtility::Log << "Writing Picture " << std::endl;
    Pane::Save(stream);

    std::cout << "writing arg count " << 3 + mPictArgs["mirror"] + mPictArgs["wrap"] + mPictArgs["fromColor"] + mPictArgs["toColor"] + mPictArgs["color0"] + mPictArgs["color1"] + mPictArgs["color2"] + mPictArgs["color3"] << std::endl; 
    stream->writeUInt8(3 + mPictArgs["mirror"] + mPictArgs["wrap"] + mPictArgs["fromColor"] + mPictArgs["toColor"] + mPictArgs["color0"] + mPictArgs["color1"] + mPictArgs["color2"] + mPictArgs["color3"]);
    mTextures[0]->Save(stream);
    mPalette.Save(stream);
    stream->writeUInt8(mBinding);
    
    if(mPictArgs["mirror"]){
        stream->writeUInt8((mMirror << 4 )| mRotate);
    }

    if(mPictArgs["wrap"]){
        stream->writeUInt8((mWrapX << 2) | mWrapY);
    }

    if(mPictArgs["fromColor"]){
        stream->writeUInt32((static_cast<uint8_t>(mFromColor.r * 255) << 24) | (static_cast<uint8_t>(mFromColor.g * 255) << 16) | (static_cast<uint8_t>(mFromColor.b * 255) << 8) | (static_cast<uint8_t>(mFromColor.a * 255)));
    }

    if(mPictArgs["toColor"]){
        stream->writeUInt32((static_cast<uint8_t>(mToColor.r * 255) << 24) | (static_cast<uint8_t>(mToColor.g * 255) << 16) | (static_cast<uint8_t>(mToColor.b * 255) << 8) | (static_cast<uint8_t>(mToColor.a * 255)));
    }

    for(int c = 0; c < 4; c++){
        if(mPictArgs[std::format("color{}", c)]){
            stream->writeUInt32((static_cast<uint8_t>(mColors[c].r * 255) << 24) | (static_cast<uint8_t>(mColors[c].g * 255) << 16) | (static_cast<uint8_t>(mColors[c].b * 255) << 8) | (static_cast<uint8_t>(mColors[c].a * 255)));
        }
    }

    std::size_t paddedEnd = Util::AlignTo(stream->tell(), 4);
    for(; stream->tell() < paddedEnd;) stream->writeUInt8(0);
}

void Window::Save(bStream::CStream* stream){
    LGenUtility::Log << "Writing Window " << std::endl;
    Pane::Save(stream);

    stream->writeUInt8(14 + mWindowArgs["contenttex"] + mWindowArgs["fromColor"] + mWindowArgs["toColor"]);
    stream->writeUInt16(mContentRect[0]);
    stream->writeUInt16(mContentRect[1]);
    stream->writeUInt16(mContentRect[2]);
    stream->writeUInt16(mContentRect[3]);

    for(std::size_t i = 0; i < 4; i++){
        mTextures[i]->Save(stream);
    }

    mPalette.Save(stream);

    uint8_t bits = 0;
    for(std::size_t i = 0; i < 4; i++){
        // fuck
        bits |= ((mTextures[i]->mMirror & 3) << (6 - (i * 2))); //mTextures[i]->mMirror = ((bits >> (6 - (i * 2))) & 3);
    }
    stream->writeUInt8(bits);

    for(std::size_t i = 0; i < 4; i++){
        stream->writeUInt32((static_cast<uint8_t>(mTextures[i]->mColor.r * 255) << 24) | (static_cast<uint8_t>(mTextures[i]->mColor.g * 255) << 16) | (static_cast<uint8_t>(mTextures[i]->mColor.b * 255) << 8) | static_cast<uint8_t>(mTextures[i]->mColor.a * 255));
    }

    if(mWindowArgs["contenttex"]){
        mContentTexture->Save(stream);
    }

    if(mWindowArgs["fromColor"]){
        stream->writeUInt32((static_cast<uint8_t>(mFromColor.r * 255) << 24) | (static_cast<uint8_t>(mFromColor.g * 255) << 16) | (static_cast<uint8_t>(mFromColor.b * 255) << 8) | (static_cast<uint8_t>(mFromColor.a * 255)));
    }

    if(mWindowArgs["toColor"]){
        stream->writeUInt32((static_cast<uint8_t>(mToColor.r * 255) << 24) | (static_cast<uint8_t>(mToColor.g * 255) << 16) | (static_cast<uint8_t>(mToColor.b * 255) << 8) | (static_cast<uint8_t>(mToColor.a * 255)));
    }

    std::size_t paddedEnd = Util::AlignTo(stream->tell(), 4);
    for(; stream->tell() < paddedEnd;) stream->writeUInt8(0);

}

void Textbox::Save(bStream::CStream* stream){
    LGenUtility::Log << "Writing Textbox " << std::endl;
    Pane::Save(stream);

    stream->writeUInt8(10 + mTextboxArgs["connectParent"] + mTextboxArgs["fromColor"] + mTextboxArgs["toColor"]);

    mFont.Save(stream);

    stream->writeUInt32((static_cast<uint8_t>(mTopColor.r * 255) << 24) | (static_cast<uint8_t>(mTopColor.g * 255) << 16) | (static_cast<uint8_t>(mTopColor.b * 255) << 8) | (static_cast<uint8_t>(mTopColor.a * 255)));
    stream->writeUInt32((static_cast<uint8_t>(mBottomColor.r * 255) << 24) | (static_cast<uint8_t>(mBottomColor.g * 255) << 16) | (static_cast<uint8_t>(mBottomColor.b * 255) << 8) | (static_cast<uint8_t>(mBottomColor.a * 255)));

    stream->writeUInt8(((mHAlign & 3) << 2) | (mVAlign & 3));

    stream->writeUInt16(mFontSpacing);
    stream->writeUInt16(mFontLeading);
    stream->writeUInt16(mFontWidth);
    stream->writeUInt16(mFontHeight);

    stream->writeUInt16(mText.size());
    stream->writeString(mText);

    if(mTextboxArgs["connectParent"]){
        stream->writeUInt8(mConnectParent);
    }

    if(mTextboxArgs["fromColor"]){
        stream->writeUInt32((static_cast<uint8_t>(mFromColor.r * 255) << 24) | (static_cast<uint8_t>(mFromColor.g * 255) << 16) | (static_cast<uint8_t>(mFromColor.b * 255) << 8) | (static_cast<uint8_t>(mFromColor.a * 255)));
    }

    if(mTextboxArgs["toColor"]){
        stream->writeUInt32((static_cast<uint8_t>(mToColor.r * 255) << 24) | (static_cast<uint8_t>(mToColor.g * 255) << 16) | (static_cast<uint8_t>(mToColor.b * 255) << 8) | (static_cast<uint8_t>(mToColor.a * 255)));
    }

    std::size_t paddedEnd = Util::AlignTo(stream->tell(), 4);
    for(; stream->tell() < paddedEnd;) stream->writeUInt8(0);

}

bool Screen::LoadBlo1(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    std::shared_ptr<Pane> prev = parent;
    while(true){
        std::size_t paneStart = stream->tell();
        
        if(stream->tell() >= stream->getSize()) return true;
        LGenUtility::Log << "[BLOIO]: Reading pane at 0x" << std::hex << stream->tell() << std::dec << std::endl;

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
            prev = std::make_shared<Textbox>();
            prev->Load(stream, parent, timg);
            if(stream->tell() != paneStart + paneLen){
                stream->seek(paneStart + paneLen);
                break;
            }
            break;
		case 0x454E4431: // END1
		case 0x45585431: // EXT1
            stream->seek(paneStart + paneLen);
            return true;
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

    int numParams = (int)stream->readUInt8() - 6;
    //std::cout << "Loading pane " << (int)numParams << " params..." << std::endl;
    mVisible = stream->readUInt8() != 0;
    stream->skip(2);

    mID = stream->readUInt32();

    mRect[0] = stream->readUInt16();
    mRect[1] = stream->readUInt16();
    mRect[2] = stream->readUInt16();
    mRect[3] = stream->readUInt16();

    //char temp[4];
    //memcpy(temp, &mID, 4);
    //std::cout << "Pane ID " << temp << " params..." << std::endl;

    if(numParams > 0){
        mPaneArgs["angle"] = true;
        mAngle = static_cast<float>(stream->readUInt16());
        numParams--;
    } else {
        mPaneArgs["angle"] = false;
        mAngle = 0;
    }

    if(numParams > 0){
        mPaneArgs["anchor"] = true;
        mAnchor = stream->readUInt8();
        numParams--;
    } else {
        mPaneArgs["anchor"] = false;
        mAnchor = (uint8_t)Anchor::TopLeft; 
    }

    if(numParams > 0){
        mPaneArgs["alpha"] = true;
        mAlpha = stream->readUInt8();
        numParams--;
    } else {
        mPaneArgs["alpha"] = false;
        mAlpha = 0xFF;
    }

    if(numParams > 0){
        mPaneArgs["inheritAlpha"] = true;
        mInheritAlpha = stream->readUInt8() != 0;
        numParams--;
    } else {
        mPaneArgs["inheritAlpha"] = false;
        mInheritAlpha = true;
    }
    
    //std::cout << "Finished Loading Pane at " << std::hex << stream->tell() << std::dec << std::endl;

    return true;
}

bool Picture::Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    Pane::Load(stream, parent, timg);
    mType = ElementType::Picture;

    std::cout << "Loading param count at " << std::hex << stream->tell() << std::dec << std::endl;
    int numParams = (int)stream->readUInt8() - 3;
    std::cout << "Loading picture " << (int)numParams << " params..." << std::endl;
    mTextures[0] = std::make_shared<Image>();
    mTextures[0]->Load(stream, timg);
    mPalette.Load(stream, timg);
    mBinding = stream->readUInt8();

    if(numParams > 0){
        uint8_t bits = stream->readUInt8();
        mMirror = bits & 3;
        mRotate = ((bits & 4) != 0);
        mPictArgs["mirror"] = true;
        numParams--;
    } else {
        mMirror = 0;
        mRotate = false;
        mPictArgs["mirror"] = false;
    }

    std::cout << (int)numParams << " params remaining..." << std::endl;

    if(numParams > 0){
        uint8_t bits = stream->readUInt8();
        mWrapX = ((bits >> 2) & 3);
        mWrapY = ((bits >> 0) & 3);
        mPictArgs["wrap"] = true;
        numParams--;
    } else {
        mPictArgs["wrap"] = false;
        mWrapX = (uint8_t)WrapMode::None;
        mWrapY = (uint8_t)WrapMode::None;
    }

    std::cout << (int)numParams << " params remaining..." << std::endl;

    if(numParams > 0){
        mPictArgs["fromColor"] = true;
        uint32_t color = stream->readUInt32();
        mFromColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
        numParams--;
    } else {
        mPictArgs["fromColor"] = false;
        mFromColor = {0.0f, 0.0f, 0.0f, 1.0f};
    }

    std::cout << (int)numParams << " params remaining..." << std::endl;

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mToColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
        mPictArgs["toColor"] = true;
        numParams--;
    } else {
        mPictArgs["toColor"] = false;
        mToColor = {1.0f, 1.0f, 1.0f, 1.0f};
    }

    std::cout << (int)numParams << " params remaining..." << std::endl;

    std::cout << "Reading Colors for " << (char*)&mID << std::endl;
    for(int c = 0; c < 4; c++){
        std::cout << "Reading Color at " << std::hex << stream->tell() << std::dec << std::endl;
        if(numParams > 0){
            uint32_t color = stream->readUInt32();
            mPictArgs[std::format("color{}", c)] = true;
            mColors[c] = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
            numParams--;
        } else {
            mPictArgs[std::format("color{}", c)] = false;
            mColors[c] = {1.0f, 1.0f, 1.0f, 1.0f};
        }
        std::cout << (int)numParams << " params remaining..." << std::endl;
    }

    stream->skip(4);
    return true;
}

bool Window::Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    Pane::Load(stream, parent, timg);
    mType = ElementType::Window;

    int numParams = (int)stream->readUInt8() - 14;

    mContentRect[0] = stream->readUInt16();
    mContentRect[1] = stream->readUInt16();
    mContentRect[2] = stream->readUInt16();
    mContentRect[3] = stream->readUInt16();

    for(std::size_t i = 0; i < 4; i++){
        mTextures[i] = std::make_shared<Image>();
        mTextures[i]->Load(stream, timg);
    }

    mPalette.Load(stream, timg);

    uint8_t bits = stream->readUInt8();
    for(std::size_t i = 0; i < 4; i++){
        mTextures[i]->mMirror = ((bits >> (6 - (i * 2))) & 3);
    }

    for(std::size_t i = 0; i < 4; i++){
        uint32_t color = stream->readUInt32();
        mTextures[i]->mColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
    }

    if(numParams > 0){
        std::cout << "Loading Context Texture at " << stream->tell() << std::endl;
        mContentTexture = std::make_shared<Image>();
        mContentTexture->Load(stream, timg);
        numParams--;
        mWindowArgs["contenttex"] = true;
    } else {
        mWindowArgs["contenttex"] = false;
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mFromColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
        mWindowArgs["fromColor"] = true;
        numParams--;
    } else {
        mFromColor = {0.0f, 0.0f, 0.0f, 0.0f};
        mWindowArgs["fromColor"] = false;
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mToColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
        mWindowArgs["toColor"] = true;
        numParams--;
    } else {
        mToColor = {1.0f, 1.0f, 1.0f, 1.0f};
        mWindowArgs["toColor"] = false;
    }

    stream->skip(4);

    return true;
}

bool Textbox::Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg){
    Pane::Load(stream, parent, timg);
    mType = ElementType::Textbox;

    int numParams = (int)stream->readUInt8() - 10;

    mFont.Load(stream, timg);

    uint32_t color = stream->readUInt32();
    mTopColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
    
    color = stream->readUInt32();
    mBottomColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};

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
        mTextboxArgs["connectParent"] = true;
        numParams--;
    } else {
        mTextboxArgs["connectParent"] = false;
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mFromColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
        mTextboxArgs["fromColor"] = true;
        numParams--;
    } else {
        mFromColor = {0.0f, 0.0f, 0.0f, 0.0f};
        mTextboxArgs["fromColor"] = false;
    }

    if(numParams > 0){
        uint32_t color = stream->readUInt32();
        mToColor = {((color >> 24) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f};
        mTextboxArgs["toColor"] = true;
        numParams--;
    } else {
        mToColor = {1.0f, 1.0f, 1.0f, 1.0f};
        mTextboxArgs["toColor"] = false;
    }

    stream->skip(4);

    return true;
}

void Window::Draw(std::shared_ptr<Blo::Pane>& selection){
    int vidx;
    bool clicked = false;

    if(mTextures[0] != nullptr && mTextures[1] != nullptr && mTextures[2] != nullptr && mTextures[3] != nullptr){
        ImGui::SetCursorPosX(mRect[0]);
        ImGui::SetCursorPosY(mRect[1]);
        ImGui::Image(static_cast<uintptr_t>(mTextures[0]->mTextureID), {static_cast<float>(mTextures[0]->mTexture.mWidth), static_cast<float>(mTextures[0]->mTexture.mHeight)}, {0.0, 0.0}, {1.0, 1.0}, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        clicked |= ImGui::IsItemClicked(0);

        ImGui::SetCursorPosX((mRect[0] + mRect[2]) - mTextures[1]->mTexture.mWidth);
        ImGui::SetCursorPosY(mRect[1]);
        ImGui::Image(static_cast<uintptr_t>(mTextures[1]->mTextureID), {static_cast<float>(mTextures[1]->mTexture.mWidth), static_cast<float>(mTextures[1]->mTexture.mHeight)}, {1.0, 0.0}, {0.0, 1.0}, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        clicked |= ImGui::IsItemClicked(0);

        ImGui::SetCursorPosX(mRect[0]);
        ImGui::SetCursorPosY((mRect[1] + mRect[3]) - mTextures[3]->mTexture.mHeight);
        ImGui::Image(static_cast<uintptr_t>(mTextures[2]->mTextureID), {static_cast<float>(mTextures[2]->mTexture.mWidth), static_cast<float>(mTextures[2]->mTexture.mHeight)}, {0.0, 1.0}, {1.0, 0.0}, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        clicked |= ImGui::IsItemClicked(0);

        ImGui::SetCursorPosX((mRect[0] + mRect[2]) - mTextures[3]->mTexture.mWidth);
        ImGui::SetCursorPosY((mRect[1] + mRect[3]) - mTextures[3]->mTexture.mHeight);
        ImGui::Image(static_cast<uintptr_t>(mTextures[3]->mTextureID), {static_cast<float>(mTextures[3]->mTexture.mWidth), static_cast<float>(mTextures[3]->mTexture.mHeight)}, {1.0, 1.0}, {0.0, 0.0}, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        clicked |= ImGui::IsItemClicked(0);

        // edges
        ImGui::SetCursorPosY(mRect[1]);
        ImGui::SetCursorPosX(mRect[0] + mTextures[1]->mTexture.mWidth);
        ImGui::Image(static_cast<uintptr_t>(mTextures[1]->mTextureID), {static_cast<float>(mRect[2] - (mTextures[1]->mTexture.mWidth*2)), static_cast<float>(mTextures[1]->mTexture.mHeight)}, {0.9999, 0.0}, {1.0, 1.0}, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        clicked |= ImGui::IsItemClicked(0);

        ImGui::SetCursorPosY(mRect[1] + mTextures[1]->mTexture.mHeight);
        ImGui::SetCursorPosX(mRect[0]);
        ImGui::Image(static_cast<uintptr_t>(mTextures[1]->mTextureID), {static_cast<float>(mTextures[3]->mTexture.mWidth), static_cast<float>(mRect[3] - (mTextures[2]->mTexture.mHeight * 2))}, {0.0, 0.9999}, {1.0, 1.0}, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        clicked |= ImGui::IsItemClicked(0);

        ImGui::SetCursorPosY(mRect[1] + mTextures[2]->mTexture.mHeight);
        ImGui::SetCursorPosX((mRect[0] + mRect[2]) - mTextures[2]->mTexture.mWidth);
        ImGui::Image(static_cast<uintptr_t>(mTextures[2]->mTextureID), {static_cast<float>(mTextures[3]->mTexture.mWidth), static_cast<float>(mRect[3] - (mTextures[2]->mTexture.mHeight * 2))}, {1.0, 0.9999}, {0.0, 1.0}, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        clicked |= ImGui::IsItemClicked(0);

        ImGui::SetCursorPosY(mRect[1] + mRect[3] - mTextures[1]->mTexture.mHeight);
        ImGui::SetCursorPosX(mRect[0] + mTextures[1]->mTexture.mWidth);
        ImGui::Image(static_cast<uintptr_t>(mTextures[1]->mTextureID), {static_cast<float>(mRect[2] - (mTextures[1]->mTexture.mWidth*2)), static_cast<float>(mTextures[1]->mTexture.mHeight)}, {0.9999, 1.0}, {1.0, 0.0}, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        clicked |= ImGui::IsItemClicked(0);

    }

    auto WindowPos = ImGui::GetWindowPos();
    ImVec2 min = {WindowPos.x + mRect[0] + mContentRect[0], WindowPos.y + mRect[1] + mContentRect[1]};
    ImVec2 max = { mContentRect[2] + min.x, mContentRect[3] + min.y};

    // draw contents
    if(mContentTexture != nullptr && mContentTexture->mTextureID != 0xFFFFFFFF){
        ImGui::SetCursorPosX(mContentRect[0]);
        ImGui::SetCursorPosY(mContentRect[1]);

        float hf = mContentRect[2] / mContentTexture->mTexture.mWidth;
        float vf = mContentRect[3] / mContentTexture->mTexture.mHeight; 

        ImGui::Image(static_cast<uintptr_t>(mContentTexture->mTextureID), {static_cast<float>(mContentRect[2]), static_cast<float>(mContentRect[3])}, {0.0, 0.0}, {hf, vf});
        clicked |= ImGui::IsItemClicked(0);
    } else {
        ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f))));
        clicked |= (ImGui::IsMouseHoveringRect(min, max) && ImGui::IsMouseClicked(0));
    }

    if(clicked){
        selection = shared_from_this();
    }

    for(auto child: mChildren){
        child->Draw(selection);
    }


}

void Picture::Draw(std::shared_ptr<Blo::Pane>& selection){
    if(mParent != nullptr){
        ImGui::SetCursorPosX(mParent->mRect[0] + mRect[0]);
        ImGui::SetCursorPosY(mParent->mRect[1] + mRect[1]);
    } else {
        ImGui::SetCursorPosX(mRect[0]);
        ImGui::SetCursorPosY(mRect[1]);
    }


    if(mVisible && mTextures[0] != nullptr && mTextures[0]->mTextureID != 0xFFFFFFFF){
        ImVec2 UV0 (0.0f, 0.0f);
        ImVec2 UV1 (1.0f, 1.0f);
        

        if(mRect[2] > mTextures[0]->mTexture.mWidth && mTextures[0]->mTexture.mWidth != 0){
            UV1.x = mRect[2] / mTextures[0]->mTexture.mWidth;
        }

        if(mRect[3] > mTextures[0]->mTexture.mHeight && mTextures[0]->mTexture.mHeight != 0){
            UV1.y = mRect[3] / mTextures[0]->mTexture.mHeight;
        }

        if(mMirror & MirrorMode::X){
            ImSwap(UV0.x, UV1.x);
        }

        if(mMirror & MirrorMode::Y){
            ImSwap(UV0.y, UV1.y);
        }

        ImGui::Image(static_cast<ImTextureID>(mTextures[0]->mTextureID), ImVec2(static_cast<float>(mRect[2]) , static_cast<float>(mRect[3])), UV0, UV1, ImVec4(mToColor.r, mToColor.g, mToColor.b, mToColor.a * (mAlpha / 255.0f)), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }
    }

    for(auto child: mChildren){
        child->Draw(selection);
    }

}

void Pane::Draw(std::shared_ptr<Blo::Pane>& selection){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));

    ImGui::SetCursorPosX(mRect[0]);
    ImGui::SetCursorPosY(mRect[1]);

    if(mParent->mRect[2] != mRect[2] && mParent->mRect[3] != mRect[3]){
        ImGui::BeginChild(std::format("Pane##pane{}", mID).c_str(), ImVec2(mRect[2], mRect[3]), ImGuiChildFlags_Border);
        ImGui::Text("Pane %4s", drawID);
        ImGui::PushID(std::format("##PreviewPane{}", mID).c_str());
        for(auto child: mChildren){
            child->Draw(selection);
        }
        ImGui::PopID();
        ImGui::EndChild();
    } else {
        ImGui::Text("Pane %4s", drawID);
        ImGui::PushID(std::format("##PreviewPane{}", mID).c_str());
        for(auto child: mChildren){
            child->Draw(selection);
        }
        ImGui::PopID();
    }
}

void Textbox::Draw(std::shared_ptr<Blo::Pane>& selection){
    ImGui::SetCursorPosX(mRect[0]);
    ImGui::SetCursorPosY(mRect[1]);

    ImVec2 textSize = ImGui::CalcTextSize(mText.c_str());

    switch((TextBoxHAlign)mHAlign){
        case TextBoxHAlign::Right:
            ImGui::SetCursorPosX(mRect[0] + (mRect[2] - textSize.x));
            break;
        case TextBoxHAlign::Center:
            ImGui::SetCursorPosX(mRect[0] + ((mRect[2] / 2) - (textSize.x / 2)));
            break;
    }

    switch((TextBoxVAlign)mVAlign){
        case TextBoxVAlign::Bottom:
            ImGui::SetCursorPosY(mRect[1] + (mRect[3] - textSize.y));
            break;
        case TextBoxVAlign::Center:
            ImGui::SetCursorPosY(mRect[1] + ((mRect[3] / 2) - (textSize.y / 2)));
            break;
    }

    ImGui::TextColored(ImColor(ImVec4(mTopColor.r, mTopColor.g, mTopColor.b, mTopColor.a)), mText.c_str());
    
    if(ImGui::IsItemClicked()){
        selection = shared_from_this();
    }
    
    //ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImGui::GetWindowDrawList(), vidx + 2, vidx + 4, {0.0, 0.0}, {0.0, 1.0}, ImColor(ImVec4(mTopColor.r, mTopColor.g, mTopColor.b, mTopColor.a)), ImColor(ImVec4(mBottomColor.r, mBottomColor.g, mBottomColor.b, mBottomColor.a)));

    ImGui::PushID(std::format("##PreviewPane{}", mID).c_str());
    for(auto child: mChildren){
        child->Draw(selection);
    }
    ImGui::PopID();
}

void Screen::Draw(std::shared_ptr<Blo::Pane>& selection){
    ImGui::BeginChild(std::format("Screen##screen{}", mID).c_str(), ImVec2(mRect[2], mRect[3]), ImGuiChildFlags_Border);

    ImGui::PushID(std::format("##PreviewScreen{}", mID).c_str());
    for(auto child: mChildren){
        child->Draw(selection);
    }
    ImGui::PopID();

    ImGui::EndChild();
}

void Screen::DrawHierarchy(std::shared_ptr<Blo::Pane>& selection){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(shared_from_this() == selection) { flags |= ImGuiTreeNodeFlags_Selected; }
    bool opened = ImGui::TreeNodeEx(std::format("Screen {}##Hierarchy{:x}", drawID, mID).c_str(), flags);
    
    if(ImGui::BeginPopupContextItem()){
        ImGui::Text("Add");
        ImGui::Separator();
        if(ImGui::Selectable("Pane")){
            mChildren.push_back(std::make_shared<Pane>());
        }
        if(ImGui::Selectable("Window")){
            mChildren.push_back(std::make_shared<Window>());
        }
        if(ImGui::Selectable("Picture")){
            mChildren.push_back(std::make_shared<Picture>());
        }
        if(ImGui::Selectable("TextBox")){
            mChildren.push_back(std::make_shared<Textbox>());
        }
        ImGui::Separator();
        if(ImGui::Selectable("Delete")){
            selection = nullptr;
            mParent->mToDelete = shared_from_this();
        }
        ImGui::EndPopup();
    }

    if(opened){

        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }

        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        ImGui::TreePop();
    }

    if(mToDelete != nullptr){
        std::erase(mChildren, mToDelete);
        mToDelete = nullptr;
    }
}

void Pane::DrawHierarchy(std::shared_ptr<Blo::Pane>& selection){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(shared_from_this() == selection) { flags |= ImGuiTreeNodeFlags_Selected; }
    bool opened = ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags);
    
    if(ImGui::BeginPopupContextItem()){
        ImGui::Text("Add");
        ImGui::Separator();
        if(ImGui::Selectable("Pane")){
            mChildren.push_back(std::make_shared<Pane>());
        }
        if(ImGui::Selectable("Window")){
            mChildren.push_back(std::make_shared<Window>());
        }
        if(ImGui::Selectable("Picture")){
            mChildren.push_back(std::make_shared<Picture>());
        }
        if(ImGui::Selectable("TextBox")){
            mChildren.push_back(std::make_shared<Textbox>());
        }
        ImGui::Separator();
        if(ImGui::Selectable("Delete")){
            selection = nullptr;
            mParent->mToDelete = shared_from_this();
        }
        ImGui::EndPopup();
    }

    if(opened){

        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }

        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        
        ImGui::TreePop();
    }

    if(mToDelete != nullptr){
        std::erase(mChildren, mToDelete);
        mToDelete = nullptr;
    }
}

void Picture::DrawHierarchy(std::shared_ptr<Blo::Pane>& selection){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(shared_from_this() == selection) { flags |= ImGuiTreeNodeFlags_Selected; }
    bool opened = ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags);
    
    if(ImGui::BeginPopupContextItem()){
        ImGui::Text("Add");
        ImGui::Separator();
        if(ImGui::Selectable("Pane")){
            mChildren.push_back(std::make_shared<Pane>());
        }
        if(ImGui::Selectable("Window")){
            mChildren.push_back(std::make_shared<Window>());
        }
        if(ImGui::Selectable("Picture")){
            mChildren.push_back(std::make_shared<Picture>());
        }
        if(ImGui::Selectable("TextBox")){
            mChildren.push_back(std::make_shared<Textbox>());
        }
        ImGui::Separator();
        if(ImGui::Selectable("Delete")){
            selection = nullptr;
            mParent->mToDelete = shared_from_this();
        }
        ImGui::EndPopup();
    }

    if(opened){

        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }

        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        
        ImGui::TreePop();
    }

    if(mToDelete != nullptr){
        std::erase(mChildren, mToDelete);
        mToDelete = nullptr;
    }
}

void Window::DrawHierarchy(std::shared_ptr<Blo::Pane>& selection){
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(shared_from_this() == selection) { flags |= ImGuiTreeNodeFlags_Selected; }
    bool opened = ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags);
    
    if(ImGui::BeginPopupContextItem()){
        ImGui::Text("Add");
        ImGui::Separator();
        if(ImGui::Selectable("Pane")){
            mChildren.push_back(std::make_shared<Pane>());
        }
        if(ImGui::Selectable("Window")){
            mChildren.push_back(std::make_shared<Window>());
        }
        if(ImGui::Selectable("Picture")){
            mChildren.push_back(std::make_shared<Picture>());
        }
        if(ImGui::Selectable("TextBox")){
            mChildren.push_back(std::make_shared<Textbox>());
        }
        ImGui::Separator();
        if(ImGui::Selectable("Delete")){
            selection = nullptr;
            mParent->mToDelete = shared_from_this();
        }
        ImGui::EndPopup();
    }

    if(opened){

        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }
        
        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        
        ImGui::TreePop();
    }

    if(mToDelete != nullptr){
        std::erase(mChildren, mToDelete);
        mToDelete = nullptr;
    }
}

void Textbox::DrawHierarchy(std::shared_ptr<Blo::Pane>& selection) {
    uint32_t id = LGenUtility::SwapEndian<uint32_t>(mID);
    char drawID[sizeof(uint32_t)] = {0};
    std::memcpy(drawID, &id, sizeof(uint32_t));
    
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked()){
        mVisible = !mVisible;
    }
    ImGui::SameLine();

    ImGuiTreeNodeFlags flags = mChildren.size() == 0 ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth : ImGuiTreeNodeFlags_None;
    if(shared_from_this() == selection) { flags |= ImGuiTreeNodeFlags_Selected; }
    
    bool opened = ImGui::TreeNodeEx(std::format("{}##Hierarchy{:x}", drawID, mID).c_str(), flags);
    
    if(ImGui::BeginPopupContextItem()){
        ImGui::Text("Add");
        ImGui::Separator();
        if(ImGui::Selectable("Pane")){
            mChildren.push_back(std::make_shared<Pane>());
        }
        if(ImGui::Selectable("Window")){
            mChildren.push_back(std::make_shared<Window>());
        }
        if(ImGui::Selectable("Picture")){
            mChildren.push_back(std::make_shared<Picture>());
        }
        if(ImGui::Selectable("TextBox")){
            mChildren.push_back(std::make_shared<Textbox>());
        }
        ImGui::Separator();
        if(ImGui::Selectable("Delete")){
            selection = nullptr;
            mParent->mToDelete = shared_from_this();
        }
        ImGui::EndPopup();
    }

    if(opened){
        if(ImGui::IsItemClicked()){
            selection = shared_from_this();
        }
        
        for(auto child: mChildren){
            child->DrawHierarchy(selection);
        }
        
        ImGui::TreePop();
    }

    if(mToDelete != nullptr){
        std::erase(mChildren, mToDelete);
        mToDelete = nullptr;
    }

}

}