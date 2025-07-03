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
        uint8_t Version { 0x02 };
        std::string Name { "bin_model"};
        uint32_t TextureOffset { 0 };     // 0
        uint32_t SamplerOffset { 0 };     // 1
        uint32_t PositionOffset { 0 };    // 2
        uint32_t NormalOffset { 0 };      // 3
        uint32_t Color0Offset { 0 };      // 4
        uint32_t Color1Offest { 0 };      // 5
        uint32_t TexCoord0Offset { 0 };   // 6
        uint32_t TexCoord1Offset { 0 };   // 7
        uint32_t UnknownOffsets[2] { 0 }; // 8, 9
        uint32_t MaterialOffset { 0 };    // 10
        uint32_t BatchOffset { 0 };       // 11
        uint32_t SceneGraphOffset { 0 };  // 12
    };
    #pragma pack(pop)

    struct DrawElement : Readable {
        int16_t MaterialIndex { -1 }, BatchIndex { -1 };

        void Read(bStream::CStream* stream) override;
    };

    struct TextureHeader : Readable {
        uint16_t Width { 0 };
        uint16_t Height { 0 };
        uint8_t Format { 0 };
        uint16_t Unknown { 0x20 };

        uint32_t ImageOffset { 0 };
        uint32_t TextureID { UINT32_MAX }; // Bind this for rendering
        uint8_t* ImageData { nullptr };

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
        void Save(bStream::CStream* stream);
        void Load(bStream::CStream* stream);

        void SetImage(uint8_t* data, std::size_t size, int w, int h);

        void Destroy();
    };

    struct Sampler : Readable {
        int16_t TextureIndex { -1 };
        int16_t PaletteIndex { -1 };;
        uint8_t WrapU { 0 };
        uint8_t WrapV { 0 };
        uint16_t Unk { 0 };

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
    };

    struct Material : Readable {
        uint8_t LightEnabled { 1 };
        uint8_t Unk0  { 0 }, Unk1 { 0 };
        glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };
        int16_t SamplerIndices[8] { -1, -1, -1, -1, -1, -1, -1, -1 };
        //uint16_t Unk2, Unk3;

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
    };

    struct Primitive {
        uint8_t Opcode;
        std::vector<Vertex> Vertices;
    };

    struct Batch : Readable {
        uint16_t TriangleCount { 0 };
        uint16_t DisplayListSize { 0 };
        uint32_t VertexAttributes { (1 << (int)GXAttribute::Position) | (1 << (int)GXAttribute::Normal) | (1 << (int)GXAttribute::Tex0) };

        uint8_t NormalFlag { 1 };
        uint8_t PositionFlag { 2 };
        uint8_t TexCoordFlag { 1 };
        uint8_t NBTFlag { 0 };

        uint32_t Vao { 0 };
        uint32_t Vbo { 0 };
        uint32_t VertexCount { 0 };
        uint32_t PrimitiveOffset { 0 };

        std::vector<Primitive> Primitives;

        void Read(bStream::CStream* stream) override;
        void Write(bStream::CStream* stream);
        void ReloadMeshes();
        void Destroy();
    };

    struct SceneGraphNode : Readable {
        int16_t Index;

        int16_t ParentIndex { -1 };
        int16_t ChildIndex { -1 };
        int16_t NextSibIndex { -1 };
        int16_t PreviousSibIndex { -1 };

        uint8_t RenderFlags { 0 };

        glm::vec3 Scale { 1.0f, 1.0f, 1.0f }, Rotation { 0.0f, 0.0f, 0.0f }, Position { 0.0f, 0.0f, 0.0f };
        glm::vec3 BoundingBoxMin { 0.0f, 0.0f, 0.0f }, BoundingBoxMax { 0.0f, 0.0f, 0.0f };
        float Radius { 0.0f };
        glm::mat4 Transform { 1.0f };

        int16_t ElementCount { 0 };
        int32_t ElementOffset { 0 };
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
    static uint32_t MissingTexID { UINT32_MAX };

    void InitShaders();
    void DestroyShaders();

    class Model
    {
    public:
        Header mHeader;

        AnimInfo mAnim;

        std::map<uint16_t, TextureHeader> mTextureHeaders;

        std::map<uint16_t, Sampler> mSamplers;
        std::map<uint16_t, Batch> mBatches;

        std::map<uint16_t, Material> mMaterials;
        std::map<uint16_t, SceneGraphNode> mGraphNodes;
        std::map<uint16_t, GraphNodeTrack> mAnimationTracks;

        std::vector<glm::vec3> mPositions;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec2> mTexCoords;
        std::vector<glm::vec4> mColors;

        void ReadSceneGraphNode(bStream::CStream* stream, uint32_t index);
        void DrawScenegraphNode(uint32_t idx, glm::mat4 transform);

        glm::vec3 bbMax {0, 0, 0}, bbMin {0, 0, 0};

        void LoadAnimation(bStream::CStream* stream);
        void ClearAnimation();

        void Draw(glm::mat4* transform, int32_t id, bool selected);

        void Load(bStream::CStream* stream);
        void Write(bStream::CStream* stream);

        static Model FromOBJ(std::string path);
        static Model FromFBX(std::string path);
		Model(bStream::CStream* stream){ Load(stream); }
        Model(){}
        ~Model();
    };

}
