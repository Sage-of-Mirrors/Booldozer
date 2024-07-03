#pragma once

#include "BGRenderDOMNode.hpp"
#include <json.hpp>
#include <vector>
#include <memory>
#include "io/CollisionIO.hpp"

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

class LMapCollisionDOMNode : public LBGRenderDOMNode
{

public:
	typedef LBGRenderDOMNode Super;

	std::vector<glm::vec3> mPositionData;
	std::vector<glm::vec3> mNormalData;
	std::vector<CollisionTriangle> mTriangles;
	std::map<std::string, std::string> mMatColProp {
		{"Group", "group"},
		{"Sound", "sound"},
		{"SoundEchoSwitch", "soundechoswitch"},
		{"Friction", "friction"},
		{"Ladder", "ladder"},
		{"IgnorePointer", "ignorepointer"},
		{"SurfaceMaterial", "surfmaterial"}
	};

	bool mWasRendered { false };
	bool mShinyFriction { true };
	glm::vec3 mGridScale, mMinBounds, mAxisLengths;

	LMapCollisionDOMNode(std::string name);

	virtual std::string GetName() override;
	virtual void RenderHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection) override;
	virtual void RenderDetailsUI(float dt) override;

	bool FromMap();

	void ImportObj(std::string path);

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
