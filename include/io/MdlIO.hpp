#pragma once
#include <io/Util.hpp>
#include <glm/glm.hpp>
#include <GenUtil.hpp>
#include "io/TxpIO.hpp"
#include "KeyframeIO.hpp"
#include "UPathRenderer.hpp"
#include <glm/gtx/quaternion.hpp>

namespace MDL {

    #pragma pack(push, 1)
    struct MDLHeader {
        uint32_t Magic { 0 };
        uint16_t FaceCount { 0 };
        uint16_t Padding { 0 };
        uint16_t SceneGraphNodeCount { 0 };
        uint16_t PacketCount { 0 };
        uint16_t WeightCount { 0 };
        uint16_t JointCount { 0 };
        uint16_t PositionCount { 0 };
        uint16_t NormalCount { 0 };
        uint16_t ColorCount { 0 };
        uint16_t TexCoordCount { 0 };
        uint8_t Padding2 { 0 }[8];
        uint16_t TextureCount { 0 };
        uint16_t Padding3 { 0 };
        uint16_t SamplerCount { 0 };
        uint16_t DrawElementCount { 0 };
        uint16_t MaterialCount { 0 };
        uint16_t ShapeCount { 0 };
        uint32_t Padding4 { 0 };
        uint32_t SceneGraphOffset { 0 };
        uint32_t PacketOffset { 0 };
        uint32_t InverseMatrixOffset { 0 };
        uint32_t WeightOffset { 0 };
        uint32_t JointIndexOffset { 0 };
        uint32_t WeightCountTableOffset { 0 };
        uint32_t PositionOffset { 0 };
        uint32_t NormalsOffset { 0 };
        uint32_t ColorsOffset { 0 };
        uint32_t TexCoordsOffset { 0 };
        uint8_t Padding5 { 0 }[8];
        uint32_t TextureOffsetArray { 0 };
        uint32_t Padding6 { 0 };
        uint32_t MaterialOffset { 0 };
        uint32_t SamplerOffset { 0 };
        uint32_t ShapeOffset { 0 };
        uint32_t DrawElementOffset { 0 };
        uint8_t Padding7 { 0 }[8];
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
        void Save(bStream::CStream* stream);
    };

    struct DrawElement : Readable {
        uint16_t MaterialIndex;
        uint16_t ShapeIndex;

        void Read(bStream::CStream* stream) override;
        void Save(bStream::CStream* stream);
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
        void Save(bStream::CStream* stream);
        void Destroy();
    };

    struct Packet : Readable {
        uint32_t DataOffset;
        uint32_t DataSize;
        uint16_t Unknown;
        uint16_t MatrixCount;
        uint16_t MatrixIndices[10];

        std::vector<Primitive> Primitives;

        void Read(bStream::CStream* stream) override;
        void Save(bStream::CStream* stream);
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
        void Save(bStream::CStream* stream);
    };

    struct Sampler : Readable {
        uint16_t TextureIndex;
        uint16_t UnknownIndex;
        uint8_t WrapU;
        uint8_t WrapV;
        uint8_t Unknown1;
        uint8_t Unknown2;

        void Read(bStream::CStream* stream) override;
        void Save(bStream::CStream* stream);
    };

    struct TextureHeader : Readable {
        uint8_t Format;
        uint8_t Padding;
        uint16_t Width;
        uint16_t Height;
        uint8_t Padding2[26];

        uint8_t* ImageData { nullptr };
        uint32_t TextureID { UINT32_MAX }; // Bind this for rendering

        void Read(bStream::CStream* stream) override;
        void Save(bStream::CStream* stream);

        void Destroy();
    };

    struct Bone {
        int32_t ParentIndex { -1 };
        Bone* Parent { nullptr };
        glm::mat4 Model { 1.0f };
        glm::mat4 Local { 1.0f };
        glm::mat4 InverseModel { 1.0f };

        glm::mat4 Transform(){
            if(Parent == nullptr){
                return Local;
            } else {
                return Parent->Transform() * Local;
            }
        }

    };

    struct Weight {
        std::vector<uint32_t> JointIndices;
        std::vector<float> Weights;
    };

    static uint32_t mProgram { UINT32_MAX };

    void InitShaders();
    void DestroyShaders();

    struct JointTrack {
        LTrackCommon ScaleX;
        LTrackCommon ScaleY;
        LTrackCommon ScaleZ;
        
        LTrackCommon RotationX;
        LTrackCommon RotationY;
        LTrackCommon RotationZ;

        LTrackCommon PositionX;
        LTrackCommon PositionY;
        LTrackCommon PositionZ;

        uint32_t mPreviousScaleKeyX { 0 };
        uint32_t mPreviousScaleKeyY { 0 };
        uint32_t mPreviousScaleKeyZ { 0 };

        uint32_t mNextScaleKeyX { 1 };
        uint32_t mNextScaleKeyY { 1 };
        uint32_t mNextScaleKeyZ { 1 };

        uint32_t mPreviousPosKeyX { 0 };
        uint32_t mPreviousPosKeyY { 0 };
        uint32_t mPreviousPosKeyZ { 0 };

        uint32_t mNextPosKeyX { 1 };
        uint32_t mNextPosKeyY { 1 };
        uint32_t mNextPosKeyZ { 1 };

        uint32_t mPreviousRotKeyX { 0 };
        uint32_t mPreviousRotKeyY { 0 };
        uint32_t mPreviousRotKeyZ { 0 };

        uint32_t mNextRotKeyX { 1 };
        uint32_t mNextRotKeyY { 1 };
        uint32_t mNextRotKeyZ { 1 };
    };

    struct AnimJoint {
        uint32_t BeginIndex;
        uint8_t Flags;
        uint8_t FrameCount;
    };

    class Animation {
    private:
        float mTime { 0.0f };
        float mSpeed { 0.1f };
        float mEnd { 0.0f };

    public:
        std::vector<JointTrack> mJointAnimations;
        void ResetTracks();
        glm::mat4 GetJoint(glm::mat4 local, uint32_t id);
        void Load(bStream::CStream* stream);
        void Step(float dt){ mTime += dt*10; if(mTime >= mEnd){ mTime = 0.0f; ResetTracks(); } }
        Animation(){}
        ~Animation(){}
    };

    class Model {
    private:

        MDLHeader mHeader;

        std::map<int, TextureHeader> mTextureHeaders;

        std::map<int, Sampler> mSamplers;
        std::map<int, Shape> mShapes;
        std::map<int, Packet> mPackets;
        std::map<int, DrawElement> mDrawElements;

        std::map<int, Material> mMaterials;
        std::map<int, SceneGraphNode> mGraphNodes;

        std::vector<glm::vec3> mPositions;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec2> mTexCoords;
        std::vector<glm::vec4> mColors;

        std::vector<glm::mat4> mMatrixTable;
        std::vector<Weight> mWeights;
        std::vector<Bone> mSkeleton;

        void BuildScenegraphSkeleton(uint32_t index, uint32_t parentIndex);
        void InitSkeletonRenderer(uint32_t index, uint32_t parentIndex);

    public:
        glm::vec3 bbMax {0, 0, 0}, bbMin {0, 0, 0};
        
        CPathRenderer mSkeletonRenderer;
        void Draw(glm::mat4* transform, int32_t id, bool selected, TXP::Animation* materialAnimtion = nullptr, Animation* skeletalAnimation = nullptr);
        void Load(bStream::CStream* stream);
        void Save(bStream::CStream* stream);

        Model(){}
        ~Model();
    };
};
