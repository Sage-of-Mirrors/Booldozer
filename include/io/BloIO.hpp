#pragma once
#include "bstream.h"
#include <vector>
#include <memory>
#include <Bti.hpp>
#include "Archive.hpp"
#include <glm/glm.hpp>

namespace Blo {
    enum class ResourceType : uint8_t {
        None,
        Unknown,
        Directory,
        Archive,
        Global
    };

    enum class WrapMode : uint8_t {
        None,
        Clamp,
        Repeat,
        Mirror
    };

    enum MirrorMode : uint8_t {
        None = 0,
        Y = 1,
        X = 2
    };

    enum class TextBoxHAlign : uint8_t {
        Center,
        Right,
        Left
    };

    enum class TextBoxVAlign : uint8_t {
        Center,
        Bottom,
        Top
    };

    enum class Binding : uint8_t {
        Bottom = (1 << 0),
        Top = (1 << 1),
        Right = (1 << 2),
        Left = (1 << 3),
    };

    enum class Anchor : uint32_t {
        TopLeft = 0,
        TopMiddle = 1,
        TopRight = 2,
        CenterLeft = 3,
        CenterMiddle = 4,
        CenterRight = 5,
        BottomLeft = 6,
        BottomMiddle = 7,
        BottomRight = 8
    };

    enum class ElementType {
        None,
        Screen,
        Pane,
        Picture,
        Window,
        Textbox
    };

    struct Resource {
        uint8_t mType;
        std::string mPath;
        virtual void Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg);
        void Save(bStream::CStream* stream);
    };

    struct Image : Resource {
        Bti mTexture;
        uint32_t mTextureID;
        uint8_t mMirror;
        glm::vec4 mColor;
        void Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg);
        ~Image();
    };

    struct Palette : Resource {
        uint8_t mPaletteType;
        uint32_t mTransparency;
        uint32_t mEntryCount;
        std::vector<uint32_t> mColors;
        void Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg);
    };

    struct Font : Resource {

    };

    class Pane : public std::enable_shared_from_this<Pane> {
    protected:
        ElementType mType;
        std::shared_ptr<Pane> mParent;
        bool mVisible;
        uint8_t mCullMode;
        uint8_t mAnchor;
        float mAngle;
        bool mInheritAlpha;
        bool mConnectParent;

        uint8_t mAccumulateAlpha;
        std::map<std::string, bool> mPaneArgs;


    public:
        std::shared_ptr<Pane> mToDelete { nullptr };

        uint32_t mID;
        int16_t mRect[4] {32, 32, 100, 100};
        uint8_t mAlpha;
        ElementType Type() { return mType; }
        std::vector<std::shared_ptr<Pane>> mChildren;
        virtual bool Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
        virtual void DrawHierarchy(std::shared_ptr<Blo::Pane>& selection);
        virtual void Draw(std::shared_ptr<Blo::Pane>& selection);
        virtual void Save(bStream::CStream* stream);

        Pane();
        Pane(ElementType t, uint32_t id);
    };

    class Picture : public Pane {
        std::shared_ptr<Image> mTextures[4] { nullptr, nullptr, nullptr, nullptr };
        Palette mPalette;
        uint8_t mBinding;
        double mBlendFactors[4];
        double mBlendAlphaFactors[4];
        int mTextureCount;
        
        uint8_t mWrapX;
        uint8_t mWrapY;
        uint8_t mMirror;
        bool mRotate;

        glm::vec4 mFromColor;
        glm::vec4 mToColor;

        glm::vec4 mColors[4];
        std::map<std::string, bool> mPictArgs;

    public:
        std::shared_ptr<Image> GetTexture(){ return mTextures[0]; }
        bool Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
        void DrawHierarchy(std::shared_ptr<Blo::Pane>& selection);
        void Draw(std::shared_ptr<Blo::Pane>& selection);
        void Save(bStream::CStream* stream);

        void SetWidth(uint16_t w) { mRect[2] = w; }
        void SetHeight(uint16_t h) { mRect[3] = h; } 

        glm::vec4* GetToColor(){ return &mToColor; }
        glm::vec4* GetFromColor(){ return &mFromColor; }

        Picture();
    };

    class Window : public Pane {
        std::shared_ptr<Image> mTextures[4] { nullptr, nullptr, nullptr, nullptr };
        std::shared_ptr<Image> mContentTexture { nullptr };
        Palette mPalette;
        
        glm::vec4 mFromColor;
        glm::vec4 mToColor;
        std::map<std::string, bool> mWindowArgs;
    public:
        int16_t mContentRect[4];
        bool Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
        void DrawHierarchy(std::shared_ptr<Blo::Pane>& selection);
        void Draw(std::shared_ptr<Blo::Pane>& selection);
        void Save(bStream::CStream* stream);

        std::shared_ptr<Image> GetContentTexture() { return mContentTexture; }

        std::shared_ptr<Image> GetTexture(uint32_t id) { if(id < 4) { return mTextures[id]; } else { return nullptr; } }
        void SetTexture(std::shared_ptr<Image> img, uint32_t id) { if(id < 4) { mTextures[id] = img; } }

        glm::vec4* GetToColor(){ return &mToColor; }
        glm::vec4* GetFromColor(){ return &mFromColor; }

        Window();
    };

    class Textbox : public Pane {
        Font mFont;
        uint8_t mHAlign, mVAlign;

        uint16_t mFontSpacing;
        uint16_t mFontLeading;
        uint16_t mFontWidth;
        uint16_t mFontHeight;

        std::string mText;

        glm::vec4 mFromColor;
        glm::vec4 mToColor;
        std::map<std::string, bool> mTextboxArgs;
    public:

        int GetFontSpacing() { return static_cast<int>(mFontSpacing); };
        int GetFontLeading() { return static_cast<int>(mFontLeading); };
        int GetFontWidth() { return static_cast<int>(mFontWidth); };
        int GetFontHeight() { return static_cast<int>(mFontHeight); };

        void SetFontSpacing(int n) { mFontSpacing = static_cast<uint16_t>(n); };
        void SetFontLeading(int n) { mFontLeading = static_cast<uint16_t>(n); };
        void SetFontWidth(int n) { mFontWidth = static_cast<uint16_t>(n); };
        void SetFontHeight(int n) { mFontHeight = static_cast<uint16_t>(n); };

        glm::vec4 mTopColor {1.0f, 1.0f, 1.0f, 1.0f}, mBottomColor {1.0f, 1.0f, 1.0f, 1.0f};
        bool Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
        void DrawHierarchy(std::shared_ptr<Blo::Pane>& selection);
        void Draw(std::shared_ptr<Blo::Pane>& selection);
        void Save(bStream::CStream* stream);

        Font* GetFont() { return &mFont; };
        std::string* GetText() { return &mText; }

        glm::vec4* GetToColor(){ return &mToColor; }
        glm::vec4* GetFromColor(){ return &mFromColor; }

        Textbox();
    };

    class Screen : public Pane {
        uint32_t mColor;
        bool LoadBlo1(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
    public:
        void Save(bStream::CStream* stream);
        bool Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg);
        void DrawHierarchy(std::shared_ptr<Blo::Pane>& selection);
        void Draw(std::shared_ptr<Blo::Pane>& selection);

        Screen();
    };

};