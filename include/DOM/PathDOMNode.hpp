#pragma once

#include "EntityDOMNode.hpp"
#include "io/JmpIO.hpp"

struct GCarchive;

struct LPathCoordinate
{
	float InTangent { 0.f };
	float Value { 0.f };
	float OutTangent { 0.f };
};

class LPathPointDOMNode : public LEntityDOMNode
{
	LPathCoordinate X;
	LPathCoordinate Y;
	LPathCoordinate Z;

	uint32_t NextRoomNumber{ 0 };

	// Hash 0x00809B31
	uint32_t UnkInt1{ 0 };
	// Hash 0x0061DCB2
	uint32_t UnkInt2{ 0 };
	// Hash 0x00D9B7C6
	uint32_t UnkInt3{ 0 };

public:
	typedef LEntityDOMNode Super;

	LPathPointDOMNode(std::string name);
	~LPathPointDOMNode() { std::cout << "Path point destroyed!" << std::endl; }

	virtual void RenderDetailsUI(float dt) override { };

	virtual void PostProcess() override { };
	virtual void PreProcess() override { };

	// Writes this path entry to the given Jmp IO object at the given index.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the given Jmp IO object at the given index into this path entry;
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::PathPoint)
			return true;

		return Super::IsNodeType(type);
	}
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

public:
	typedef LEntityDOMNode Super;

	LPathDOMNode(std::string name);
	~LPathDOMNode() { std::cout << "Path destroyed!" << std::endl; }

	size_t GetNumPoints() { return Children.size(); }

	virtual void RenderDetailsUI(float dt) override;

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

	void PostProcess(const GCarchive& mapArc);
	void PreProcess(LJmpIO& pathJmp, bStream::CMemoryStream& pathStream);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Path)
			return true;

		return Super::IsNodeType(type);
	}
};
