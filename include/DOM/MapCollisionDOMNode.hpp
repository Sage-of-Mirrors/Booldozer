#pragma once

#include "BGRenderDOMNode.hpp"
#include <json.hpp>

namespace bStream
{
	class CFileStream;
	class CMemoryStream;
}

struct LCollisionTriangle {
	int16_t positionIdx[3];
	int16_t normalIdx;
	int16_t edgeTanIdx[3];
	int16_t unkIdx;
	float dot;
	int16_t enabledMask;
	int16_t friction;
};

class LMapCollisionDOMNode : public LBGRenderDOMNode
{
private:

	glm::vec3 gridScale, minBounds, axisLengths;

	std::vector<glm::vec3> positionData;
	std::vector<glm::vec3> normalData;

	std::vector<LCollisionTriangle> triangles;
public:
	typedef LBGRenderDOMNode Super;

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
