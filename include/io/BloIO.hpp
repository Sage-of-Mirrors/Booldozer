#pragma once
#include "bstream.h"
#include <vector>
#include <memory>
#include "io/BtiIO.hpp"
#include "Archive.hpp"
#include <glm/glm.hpp>

namespace Blo {
    enum class ResourceType : uint32_t {
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
        Left = (1 << 3)
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
        ResourceType mType;
        std::string mPath;
        virtual void Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg);
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
        std::vector<std::shared_ptr<Pane>> mChildren;
        uint32_t mID;
        bool mVisible;
        uint8_t mCullMode;
        Anchor mAnchor;
        float mAngle;
        uint8_t mAplha;
        bool mInheritAlpha;
        bool mConnectParent;

        uint8_t mAccumulateAlpha;

    public:

        ElementType Type() { return mType; }
        int16_t mRect[4];
        virtual bool Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
        virtual void DrawHierarchy(std::shared_ptr<Blo::Pane> selection);
        virtual void Draw();
    };

    class Picture : public Pane {
        Image mTexutres[4];
        Palette mPalette;
        Binding mBinding;
        double mBlendFactors[4];
        double mBlendAlphaFactors[4];
        int mTextureCount;
        
        WrapMode mWrapX;
        WrapMode mWrapY;
        uint8_t mMirror;
        bool mRotate;

        glm::vec4 mFromColor;
        glm::vec4 mToColor;

        glm::vec4 mColors[4];

    public:
        bool Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
        void DrawHierarchy(std::shared_ptr<Blo::Pane> selection);
        void Draw();
    };

    class Window : public Pane {
        Image mTextures[4];
        Image mContentTexture;
        Palette mPalette;
        Bti mContectBG;
        
        int16_t mContentRect[4];
        glm::vec4 mFromColor;
        glm::vec4 mToColor;
    public:
        bool Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
        void DrawHierarchy(std::shared_ptr<Blo::Pane> selection);
        void Draw();
    };

    class Textbox : public Pane {
        Font mFont;
        glm::vec4 mTopColor, mBottomColor;
        uint8_t mHAlign, mVAlign;

        uint16_t mFontSpacing;
        uint16_t mFontLeading;
        uint16_t mFontWidth;
        uint16_t mFontHeight;

        std::string mText;

        glm::vec4 mFromColor;
        glm::vec4 mToColor;
    public:
        bool Load(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
        void DrawHierarchy(std::shared_ptr<Blo::Pane> selection);
        void Draw();
    };

    class Screen : public Pane {
        uint32_t mColor;
        bool LoadBlo1(bStream::CStream* stream, std::shared_ptr<Pane> parent, std::shared_ptr<Archive::Folder> timg);
    public:
        bool Load(bStream::CStream* stream, std::shared_ptr<Archive::Folder> timg);
        void DrawHierarchy(std::shared_ptr<Blo::Pane> selection);
        void Draw();
    };

};