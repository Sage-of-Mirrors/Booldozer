#pragma once

#include "EntityDOMNode.hpp"

struct GCarchive;

struct PathCoordinate
{
	float InTangent { 0.f };
	float Value { 0.f };
	float OutTangent { 0.f };
};

struct PathPoint
{
	PathCoordinate X;
	PathCoordinate Y;
	PathCoordinate Z;

	uint32_t NextRoomNumber{ 0 };

	// Hash 0x00809B31
	uint32_t UnkInt1{ 0 };
	// Hash 0x0061DCB2
	uint32_t UnkInt2{ 0 };
	// Hash 0x00D9B7C6
	uint32_t UnkInt3{ 0 };
};

class LPathDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mInterpolationType { "(null)" };
	std::string mNextPathName { "(null)" };

	int32_t mOrganizationNumber { 0 };
	int32_t mNumPoints { 0 };
	int32_t mArg0 { 0 };
	int32_t mDoType { 0 };

	bool mIsClosed { false };
	bool mUse { false };

	std::vector<PathPoint> mPoints;

public:
	typedef LEntityDOMNode Super;

	LPathDOMNode(std::string name);

	virtual void RenderDetailsUI(float dt) override;

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

	void PostProcess(const GCarchive& mapArc);

/*=== Type operations ===*/

	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Path)
			return true;

		return Super::IsNodeType(type);
	}
};
