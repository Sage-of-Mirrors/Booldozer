#pragma once

#include <map>
#include <memory>
#include <bstream.h>
#include <vector>
#include "Util.hpp"
#include "io/KeyframeIO.hpp"

namespace BIN {

    struct AnimInfo {
    	bool mLoop { 0 };
    	bool mPlaying { false };
    	bool mLoaded { false };
    	float mCurrentFrame { 0.0f }; // float so that we can change speed more easily
    	float mPlaybackSpeed { 0.5f };
    	uint32_t mFrameCount { 0 };
    };

    #pragma pack(push, 1)
    struct Header {
        uint8_t Version;
        std::string Name;
        uint32_t TextureOffset;     // 0
        uint32_t SamplerOffset;     // 1
        uint32_t PositionOffset;    // 2
        uint32_t NormalOffset;      // 3
        uint32_t Color0Offset;      // 4
        uint32_t Color1Offest;      // 5
        uint32_t TexCoord0Offset;   // 6
        uint32_t TexCoord1Offset;   // 7
        uint32_t UnknownOffsets[2]; // 8, 9
        uint32_t MaterialOffset;    // 10
        uint32_t BatchOffset;       // 11
        uint32_t SceneGraphOffset;  // 12
    };
    #pragma pack(pop)

    struct DrawElement : Readable {
        int16_t MaterialIndex, BatchIndex;

        void Read(bStream::CStream* stream) override;
    };

    struct TextureHeader : Readable {
        uint16_t Width;
        uint16_t Height;
        uint8_t Format;
        uint16_t Unknown;

        uint32_t ImageOffset;
        uint32_t TextureID { UINT32_MAX }; // Bind this for rendering
        uint8_t* ImageData;

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
        void Save(bStream::CStream* stream);
        void Load(bStream::CStream* stream);

        void SetImage(uint8_t* data, std::size_t size, int w, int h);

        void Destroy();
    };

    struct Sampler : Readable {
        int16_t TextureIndex;
        uint16_t PaletteIndex;
        uint8_t WrapU;
        uint8_t WrapV;
        uint16_t Unk;

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
    };

    struct Material : Readable {
        uint8_t LightEnabled;
        uint8_t Unk0, Unk1;
        glm::vec4 Color;
        int16_t SamplerIndices[8];
        //uint16_t Unk2, Unk3;

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
    };

    struct Primitive {
        uint8_t Opcode;
        std::vector<Vertex> Vertices;
    };

    struct Batch : Readable {
        uint16_t TriangleCount;
        uint16_t DisplayListSize;
        uint32_t VertexAttributes;

        uint8_t NormalFlag;
        uint8_t PositionFlag;
        uint8_t TexCoordFlag;
        uint8_t NBTFlag;

        uint32_t Vao;
        uint32_t Vbo;
        uint32_t VertexCount;
        uint32_t PrimitiveOffset;

        std::vector<Primitive> Primitives;

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
        void Destroy();
    };

    struct SceneGraphNode : Readable {
        int16_t Index;

        int16_t ParentIndex { -1 };
        int16_t ChildIndex { -1 };
        int16_t NextSibIndex { -1 };
        int16_t PreviousSibIndex { -1 };

        uint8_t RenderFlags;

        glm::vec3 Scale, Rotation, Position;
        glm::vec3 BoundingBoxMin, BoundingBoxMax;
        float Radius;
        glm::mat4 Transform { 1.0f };

        int16_t ElementCount;
        int32_t ElementOffset;
        std::vector<DrawElement> mDrawElements;

        inline bool CastShadow() { return (RenderFlags & (1 << 1)) != 0; }
        inline bool FourthWall() { return (RenderFlags & (1 << 2)) != 0; }
        inline bool Transparent() { return (RenderFlags & (1 << 3)) != 0; }
        inline bool FullBright() { return (RenderFlags & (1 << 6)) != 0; }
        inline bool Ceiling() { return (RenderFlags & (1 << 7)) != 0; }

        inline bool CastShadow(bool set) { if (set) { RenderFlags |= (1 << 1); } else { RenderFlags &= ~(1 << 1); } return set; }
        inline bool FourthWall(bool set) { if (set) { RenderFlags |= (1 << 2); } else { RenderFlags &= ~(1 << 2); } return set; }
        inline bool Transparent(bool set) { if (set) { RenderFlags |= (1 << 3); } else { RenderFlags &= ~(1 << 3); } return set; }
        inline bool FullBright(bool set) { if (set) { RenderFlags |= (1 << 6); } else { RenderFlags &= ~(1 << 6); } return set; }
        inline bool Ceiling(bool set) { if (set) { RenderFlags |= (1 << 7); } else { RenderFlags &= ~(1 << 7); } return set; }

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
    };

    struct GraphNodeTrack {
        LTrackCommon mXScaleTrack;
        LTrackCommon mYScaleTrack;
        LTrackCommon mZScaleTrack;

        LTrackCommon mXRotTrack;
        LTrackCommon mYRotTrack;
        LTrackCommon mZRotTrack;

        LTrackCommon mXPosTrack;
        LTrackCommon mYPosTrack;
        LTrackCommon mZPosTrack;

        uint32_t mNextScaleKeyX { 1 };
        uint32_t mNextScaleKeyY { 1 };
        uint32_t mNextScaleKeyZ { 1 };

        uint32_t mNextPosKeyX { 1 };
        uint32_t mNextPosKeyY { 1 };
        uint32_t mNextPosKeyZ { 1 };

        uint32_t mNextRotKeyX { 1 };
        uint32_t mNextRotKeyY { 1 };
        uint32_t mNextRotKeyZ { 1 };
    };

    static uint32_t mProgram { UINT32_MAX };

    void InitShaders();
    void DestroyShaders();

    class Model
    {
    public:
        Header mHeader;

        AnimInfo mAnim;

        std::map<uint16_t, TextureHeader> mTexturesHeaders;

        std::map<uint16_t, Sampler> mSamplers;
        std::map<uint16_t, Batch> mBatches;

        std::map<uint16_t, Material> mMaterials;
        std::map<uint16_t, SceneGraphNode> mGraphNodes;
        std::map<uint16_t, GraphNodeTrack> mAnimationTracks;

        std::vector<glm::vec3> mPositions;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec2> mTexCoords;

        void ReadSceneGraphNode(bStream::CStream* stream, uint32_t index);
        void DrawScenegraphNode(uint32_t idx, glm::mat4 transform);

        glm::vec3 bbMax {0, 0, 0}, bbMin {0, 0, 0};

        void LoadAnimation(bStream::CStream* stream);
        void ClearAnimation();

        void Draw(glm::mat4* transform, int32_t id, bool selected);

        void Load(bStream::CStream* stream);
        void Write(bStream::CStream* stream);

		Model(bStream::CStream* stream){ Load(stream); }
        Model(){}
        ~Model();
    };

}
