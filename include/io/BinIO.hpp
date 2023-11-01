#pragma once

#include <map>
#include <memory>
#include "../lib/bStream/bstream.h"

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

class BinModel;

class BinScenegraphNode {
    std::vector<std::pair<int16_t, int16_t>> meshes;


public:
    std::shared_ptr<BinScenegraphNode> parent;
    std::shared_ptr<BinScenegraphNode> child;
    std::shared_ptr<BinScenegraphNode> next;
    std::shared_ptr<BinScenegraphNode> prev;
    glm::mat4 transform;
	float mBoundingSphereRadius;

    void AddMesh(int16_t material, int16_t mesh);
    void Draw(glm::mat4 localTransform, glm::mat4* instance, BinModel* bin, bool bIgnoreTransforms = false);
    BinScenegraphNode();
    ~BinScenegraphNode();

};

class BinModel {
    std::map<int16_t, std::shared_ptr<BinMesh>> mMeshes;
    std::map<int16_t, std::shared_ptr<BinSampler>> mSamplers;
	std::vector<std::shared_ptr<BinMaterial>> mMaterials;
    std::shared_ptr<BinScenegraphNode> mRoot;

    std::shared_ptr<BinScenegraphNode> ParseSceneraph(bStream::CStream* stream, uint32_t* offsets, uint16_t index, std::vector<glm::vec3>& vertexData, std::vector<glm::vec2>& texcoordData, std::shared_ptr<BinScenegraphNode> parent = nullptr, std::shared_ptr<BinScenegraphNode> previous = nullptr);

public:

	static void InitShaders();
	static void DestroyShaders();

	bool BindMesh(uint16_t id);

	bool BindMaterial(uint16_t id);

    void Draw(glm::mat4* transform, int32_t id, bool selected, bool bIgnoreTransforms = false);

	void TranslateRoot(glm::vec3 translation);

	float GetRootBoundingSphere() { return mRoot->mBoundingSphereRadius; }

	std::shared_ptr<BinMesh> GetMesh(uint32_t id) { return mMeshes.at(id); }
	std::shared_ptr<BinSampler> GetSampler(uint32_t id) { return mSamplers.at(id); }

    BinModel(bStream::CStream* stream);

    ~BinModel();
};