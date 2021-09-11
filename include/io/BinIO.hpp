#pragma once

#include <map>
#include <memory>
#include "bgfx/bgfx.h"
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
	bgfx::TextureHandle mTexture;
    uint8_t mWrap;
	void LoadCMPRTex(bStream::CStream* stream, uint16_t w, uint16_t h, uint32_t* out);

public:
    void bind(bgfx::UniformHandle& textureUniform);
	BinMaterial(bStream::CStream* stream, uint32_t textureOffset);
	~BinMaterial(){}
};

class BinSampler {
	uint32_t mAmbientColor;

public:
	uint16_t mTextureID;
    BinSampler(bStream::CStream* stream);
	BinSampler(){}
    ~BinSampler(){}
};

class BinMesh {
    bgfx::IndexBufferHandle ebo;
    bgfx::VertexBufferHandle vbo;

public:
    void bind();
    BinMesh(bStream::CStream* stream, uint32_t offset, std::vector<glm::vec3>& vertexData, std::vector<glm::vec2>& texcoordData);
    BinMesh(){}
    ~BinMesh();

};

class BGFXBin;

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
    void Draw(glm::mat4 localTransform, glm::mat4* instance, BGFXBin* bin, bgfx::ProgramHandle& program, bgfx::UniformHandle& texUniform);
    BinScenegraphNode();
    ~BinScenegraphNode();

};

class BGFXBin {
    std::map<int16_t, BinMesh> mMeshes;
    std::map<int16_t, BinSampler> mSamplers;
	std::vector<BinMaterial> mMaterials;
    std::shared_ptr<BinScenegraphNode> mRoot;

    std::shared_ptr<BinScenegraphNode> ParseSceneraph(bStream::CStream* stream, uint32_t* offsets, uint16_t index, std::vector<glm::vec3>& vertexData, std::vector<glm::vec2>& texcoordData, std::shared_ptr<BinScenegraphNode> parent = nullptr, std::shared_ptr<BinScenegraphNode> previous = nullptr);

public:
    static void InitBinVertex();
	bool BindMesh(uint16_t id);
	bool BindMaterial(uint16_t id, bgfx::UniformHandle& texUniform);
    void Draw(glm::mat4* transform, bgfx::ProgramHandle& program, bgfx::UniformHandle& texUniform);
	void TranslateRoot(glm::vec3 translation);
	float GetRootBoundingSphere() { return mRoot->mBoundingSphereRadius; }
    BGFXBin(bStream::CStream* stream);
    ~BGFXBin();
};