#pragma once

#include "BGRenderDOMNode.hpp"
#include <json.hpp>
#include <vector>
#include <memory>

namespace bStream
{
	class CFileStream;
	class CMemoryStream;
}

struct LCollisionTriangle {
	std::string name;
	int16_t positionIdx[3];
	int16_t normalIdx;
	int16_t edgeTanIdx[3];
	int16_t unkIdx;
	float dot;
	int16_t enabledMask;
	int16_t friction;
};

struct LTriangleGroup {
	std::string name;
	uint32_t startOffset;
	uint32_t endOffset;
	bool render { false };
	std::vector<std::weak_ptr<LCollisionTriangle>> triangles;
};

struct LCollisionGridCell {
	std::weak_ptr<LTriangleGroup> allTriangles;
	std::weak_ptr<LTriangleGroup> floorTriangles;
};


class LMapCollisionDOMNode : public LBGRenderDOMNode
{
private:
	int W1 { 1 }, W2 { 1 };


	std::vector<glm::vec3> mNormalData;

	std::vector<std::shared_ptr<LCollisionTriangle>> mTriangles;


public:
	typedef LBGRenderDOMNode Super;

	std::vector<glm::vec3> mPositionData;
	std::vector<std::shared_ptr<LTriangleGroup>> mTriangleGroups;

	bool mDirty { false };
	bool mGridRender { true };
	int mGridYLevel { 0 };
	uint32_t mGridDimension[3];
	std::shared_ptr<LCollisionGridCell> mSelectedCell { nullptr };
	glm::vec3 mGridScale, mMinBounds, mAxisLengths;
	std::vector<std::shared_ptr<LCollisionGridCell>> mGrid;

	LMapCollisionDOMNode(std::string name);

	virtual std::string GetName() override;
	virtual void RenderDetailsUI(float dt) override;

	bool Load(bStream::CMemoryStream* stream);
	bool Save(bStream::CMemoryStream* stream);

	void PostProcess();
	void PreProcess();


/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::MapCollision)
			return true;

		return Super::IsNodeType(type);
	}
};
