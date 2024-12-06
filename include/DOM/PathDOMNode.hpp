#pragma once

#include "EntityDOMNode.hpp"
#include "io/JmpIO.hpp"
#include "UPathRenderer.hpp"
#include <Archive.hpp>

class LRoomDOMNode;

extern std::map<std::string, std::string> InterpolationTypes;

enum class EPathDoType
{
	None,
	Path_Loop_Start = 4,
	Path_Loop_Middle = 5,
	Path_Loop_End = 6
};

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

	uint32_t mNextRoomNumber{ 0 };

	// Hash 0x00809B31
	int32_t mUnkInt1{ 0 };
	// Hash 0x0061DCB2
	int32_t mUnkInt2{ 0 };
	// Hash 0x00D9B7C6
	int32_t mUnkInt3{ 0 };

	std::weak_ptr<LRoomDOMNode> mRoomRef;

public:
	typedef LEntityDOMNode Super;

	LPathPointDOMNode(std::string name);
	~LPathPointDOMNode() { /*LGenUtility::Log << "Path point destroyed!" << std::endl;*/ }

	virtual void RenderDetailsUI(float dt) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

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
	EPathDoType mDoType { EPathDoType::None };

	bool mIsClosed { false };
	bool mUse { false };

	std::weak_ptr<LPathDOMNode> mNextPathRef;


public:
	typedef LEntityDOMNode Super;

	LPathDOMNode(std::string name);
	~LPathDOMNode() { /*LGenUtility::Log << "Path destroyed!" << std::endl;*/ }
	
	glm::vec4 mPathColor;

	size_t GetNumPoints() { return Children.size(); }

	virtual void RenderDetailsUI(float dt) override;
	
	std::vector<CPathPoint> mPathRenderable;

	void RenderPath(CPathRenderer* Renderer);

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

	void PostProcess(std::shared_ptr<Archive::Rarc> mapArchive);
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
