#pragma once

#include <map>
#include <memory>
#include "GXGeometry.hpp"
#include "../lib/bStream/bstream.h"

class BinMaterial {
    uint8_t mWrap[2];
	uint8_t* mTextureData;

public:
    void bind();
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
    std::map<int16_t, GXShape*> mMeshes;
    std::map<int16_t, BinSampler> mSamplers;
	std::vector<BinMaterial> mMaterials;
    std::shared_ptr<BinScenegraphNode> mRoot;

	GXAttributeData mVertexData;

    std::shared_ptr<BinScenegraphNode> ParseSceneraph(bStream::CStream* stream, uint32_t* offsets, uint16_t index, std::shared_ptr<BinScenegraphNode> parent, std::shared_ptr<BinScenegraphNode> previous = nullptr);

public:
    void Draw(glm::mat4* transform, bool bIgnoreTransforms = false);
	void TranslateRoot(glm::vec3 translation);
	float GetRootBoundingSphere() { return mRoot->mBoundingSphereRadius; }
    BinModel(bStream::CStream* stream);
    ~BinModel();
};