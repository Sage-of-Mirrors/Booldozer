#pragma once
#include <io/Util.hpp>
#include <glm/glm.hpp>
#include <GenUtil.hpp>
#include "io/TxpIO.hpp"

namespace MDL {

    #pragma pack(push, 1)
    struct MDLHeader {
        uint32_t Magic;
        uint16_t FaceCount;
        uint16_t Padding;
        uint16_t SceneGraphNodeCount;
        uint16_t PacketCount;
        uint16_t WeightCount;
        uint16_t JointCount;
        uint16_t PositionCount;
        uint16_t NormalCount;
        uint16_t ColorCount;
        uint16_t TexCoordCount;
        uint8_t Padding2[8];
        uint16_t TextureCount;
        uint16_t Padding3;
        uint16_t SamplerCount;
        uint16_t DrawElementCount;
        uint16_t MaterialCount;
        uint16_t ShapeCount;
        uint32_t Padding4;
        uint32_t SceneGraphOffset;
        uint32_t PacketOffset;
        uint32_t InverseMatrixOffset;
        uint32_t WeightOffset;
        uint32_t JointIndexOffset;
        uint32_t WeightCountTableOffset;
        uint32_t PositionOffset;
        uint32_t NormalsOffset;
        uint32_t ColorsOffset;
        uint32_t TexCoordsOffset;
        uint8_t Padding5[8];
        uint32_t TextureOffsetArray;
        uint32_t Padding6;
        uint32_t MaterialOffset;
        uint32_t SamplerOffset;
        uint32_t ShapeOffset;
        uint32_t DrawElementOffset;
        uint8_t Padding7[8];
    };
    #pragma pack(pop)

    struct Readable {
        virtual void Read(bStream::CStream* stream) = 0;
        virtual ~Readable(){}
    };

    struct SceneGraphNode : Readable {
        uint16_t InverseMatrixIndex;
        uint16_t ChildIndexShift;
        uint16_t SiblingIndexShift;
        uint16_t PaddingFirst;
        uint16_t DrawElementCount;
        uint16_t DrawElementBeginIndex;
        uint32_t PaddingSecond;

        void Read(bStream::CStream* stream) override;
    };

    struct DrawElement : Readable {
        uint16_t MaterialIndex;
        uint16_t ShapeIndex;

        void Read(bStream::CStream* stream) override;
    };

    struct Shape : Readable {
        uint8_t NormalFlag;
        uint8_t Unknown;
        uint8_t SurfaceFlags;
        uint8_t UnknownFlag;
        uint16_t PacketCount;
        uint16_t PacketBeginIndex;

        uint32_t Vao, Vbo, VertexCount; // Bind these for rendering

        void Read(bStream::CStream* stream) override;
        void Destroy();
    };

    struct Packet : Readable {
        uint32_t DataOffset;
        uint32_t DataSize;
        uint16_t Unknown;
        uint16_t MatrixCount;
        uint16_t MatrixIndices[10];


        void Read(bStream::CStream* stream) override;
        void Destroy(); // Delete arrays
    };

    struct TevStage {
        uint16_t Unknown;
        uint16_t SamplerIndex;
        float UnknownFloats[7];
    };

    struct Material : Readable {
        glm::vec4 DiffuseColor;
        uint16_t Unknown;
        uint8_t AlphaFlag;
        uint8_t TevStageCount;
        uint8_t Unknown2;
        uint8_t Padding[23];
        std::vector<TevStage> TevStages;

        void Read(bStream::CStream* stream) override;
    };

    struct Sampler : Readable {
        uint16_t TextureIndex;
        uint16_t UnknownIndex;
        uint8_t WrapU;
        uint8_t WrapV;
        uint8_t Unknown1;
        uint8_t Unknown2;

        void Read(bStream::CStream* stream) override;
    };

    struct TextureHeader : Readable {
        uint8_t Format;
        uint8_t Padding;
        uint16_t Width;
        uint16_t Height;
        uint8_t Padding2[26];

        uint32_t TextureID { UINT32_MAX }; // Bind this for rendering

        void Read(bStream::CStream* stream) override;

        void Destroy();
    };

    struct Weight {
        std::vector<uint32_t> JointIndices;
        std::vector<float> Weights;
    };

    static uint32_t mProgram { UINT32_MAX };

    void InitShaders();
    void DestroyShaders();

    class Model
    {
    private:

        MDLHeader mHeader;

        std::vector<TextureHeader> mTextureHeaders;

        std::vector<Sampler> mSamplers;
        std::vector<Shape> mShapes;
        std::vector<Packet> mPackets;
        std::vector<DrawElement> mDrawElements;

        std::vector<Material> mMaterials;
        std::vector<SceneGraphNode> mGraphNodes;

        std::vector<glm::vec3> mPositions;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec2> mTexCoords;
        std::vector<glm::vec4> mColors;

        std::vector<glm::mat4> mMatrixTable;
        std::vector<Weight> mWeights;


    public:
        glm::vec3 bbMax {0, 0, 0}, bbMin {0, 0, 0};

        void Draw(glm::mat4* transform, int32_t id, bool selected, TXP::Animation* materialAnimtion);

        void Load(bStream::CStream* stream);

        Model(){}
        ~Model();
    };
};
