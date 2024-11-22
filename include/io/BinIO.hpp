#pragma once

#include <map>
#include <memory>
#include "../lib/bStream/bstream.h"
#include "io/KeyframeIO.hpp"

enum class GXAttribute : int {
	PositionMatrixIndex,
	Tex0MatrixIndex,
	Tex1MatrixIndex,
	Tex2MatrixIndex,
	Tex3MatrixIndex,
	Tex4MatrixIndex,
	Tex5MatrixIndex,
	Tex6MatrixIndex,
	Tex7MatrixIndex,
	Position,
	Normal,
	Color0,
	Color1,
	Tex0,
	Tex1,
	Tex2,
	Tex3,
	Tex4,
	Tex5,
	Tex6,
	Tex7,
	PositionMatrixArray,
	NormalMatrixArray,
	TextureMatrixArray,
	LitMatrixArray,
	NormalBinormalTangent,
	NullAttr = 0xFF
};

enum GXPrimitiveType {
	Points = 0xB8,
	Lines = 0xA8,
	LineStrip = 0xB0,
	Triangles = 0x90,
	TriangleStrip = 0x98,
	TriangleFan = 0xA0,
	Quads = 0x80,
	PrimitiveNone = 0x00
};

class BinMaterial {
	uint32_t mTexture;
    uint8_t mWrap;

public:

	static void DecodeCMPR(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
	static void DecodeRGB5A3(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
	static void DecodeRGB565(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);

	static uint8_t* DecodeCMPRSubBlock(bStream::CStream* stream);
	static uint32_t RGB565toRGBA8(uint16_t data);
	static uint32_t RGB5A3toRGBA8(uint16_t data);

    void Bind();
	BinMaterial(bStream::CStream* stream, uint32_t textureOffset);
	~BinMaterial();
};

class BinSampler {

public:
	glm::vec4 mAmbientColor;
	uint16_t mTextureID;
    BinSampler(bStream::CStream* stream);
	BinSampler(){}
    ~BinSampler(){}
};

class BinMesh {
    uint32_t mVbo;
	uint32_t mVao;

public:
	uint32_t mVertexCount;

    void Bind();

    BinMesh(bStream::CStream* stream, uint32_t offset, std::vector<glm::vec3>& vertexData, std::vector<glm::vec2>& texcoordData);
	
    BinMesh(){}
    ~BinMesh();

};

struct BinAnimInfo {
	bool mLoop { 0 };
	bool mPlaying { false };
	bool mLoaded { false };
	float mCurrentFrame { 0.0f }; // float so that we can change speed more easily
	float mPlaybackSpeed { 0.5f };
	uint32_t mFrameCount { 0 };
};

class BinModel;

class BinScenegraphNode {
	friend BinModel;
    std::vector<std::pair<int16_t, int16_t>> meshes;

	void LoadNodeTracks(bStream::CStream* stream, uint32_t& idx, uint32_t groupOffset, uint32_t scaleKeysOffset, uint32_t rotateKeysOffset, uint32_t translateKeysOffset);

	uint32_t mNextScaleKeyX { 1 };
    uint32_t mNextScaleKeyY { 1 };
    uint32_t mNextScaleKeyZ { 1 };
	LTrackCommon mXScaleTrack;
	LTrackCommon mYScaleTrack;
	LTrackCommon mZScaleTrack;

	uint32_t mNextRotKeyX { 1 };
    uint32_t mNextRotKeyY { 1 };
    uint32_t mNextRotKeyZ { 1 };
	LTrackCommon mXRotTrack;
	LTrackCommon mYRotTrack;
	LTrackCommon mZRotTrack;

	uint32_t mNextPosKeyX { 1 };
    uint32_t mNextPosKeyY { 1 };
    uint32_t mNextPosKeyZ { 1 };
	LTrackCommon mXPosTrack;
	LTrackCommon mYPosTrack;
	LTrackCommon mZPosTrack;
public:
    std::shared_ptr<BinScenegraphNode> parent;
    std::shared_ptr<BinScenegraphNode> child;
    std::shared_ptr<BinScenegraphNode> next;
    std::shared_ptr<BinScenegraphNode> prev;
    

	glm::mat4 transform;
	float mBoundingSphereRadius;

	void ResetAnimation();

    void AddMesh(int16_t material, int16_t mesh);
    void Draw(glm::mat4 localTransform, glm::mat4* instance, BinModel* bin, bool ignoreTransforms = false, bool animate = false);
    BinScenegraphNode();
    ~BinScenegraphNode();

};

class BinModel {
	friend BinScenegraphNode;
    std::map<int16_t, std::shared_ptr<BinMesh>> mMeshes;
    std::map<int16_t, std::shared_ptr<BinSampler>> mSamplers;
	std::vector<std::shared_ptr<BinMaterial>> mMaterials;
    std::shared_ptr<BinScenegraphNode> mRoot;

    std::shared_ptr<BinScenegraphNode> ParseSceneraph(bStream::CStream* stream, uint32_t* offsets, uint16_t index, std::vector<glm::vec3>& vertexData, std::vector<glm::vec2>& texcoordData, std::shared_ptr<BinScenegraphNode> parent = nullptr, std::shared_ptr<BinScenegraphNode> previous = nullptr);


public:
	BinAnimInfo mAnimationInformation;

	static void InitShaders();
	static void DestroyShaders();

	bool BindMesh(uint16_t id);

	bool BindMaterial(uint16_t id);

    void Draw(glm::mat4* transform, int32_t id, bool selected, bool ignoreTransforms = false, bool animate = false);

	void ResetAnimation(){ mRoot->ResetAnimation(); }

	void TranslateRoot(glm::vec3 translation);

	float GetRootBoundingSphere() { return mRoot->mBoundingSphereRadius; }

	glm::vec3 GetRootPosition() { return mRoot->transform[3]; }

	void LoadAnimation(bStream::CStream* stream);
	void ClearAnimation();

	std::shared_ptr<BinMesh> GetMesh(uint32_t id) { return mMeshes.at(id); }
	std::shared_ptr<BinSampler> GetSampler(uint32_t id) { return mSamplers.at(id); }

    BinModel(bStream::CStream* stream);

    ~BinModel();
};