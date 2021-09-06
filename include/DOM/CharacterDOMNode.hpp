#pragma once

#include "EntityDOMNode.hpp"

enum class EConditionType : uint32_t;
enum class EAppearType : uint32_t;
class LItemAppearDOMNode;

enum class EAttackType : uint32_t
{
	None,
	Idle = 3
};

class LCharacterDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mCreateName;
	std::string mPathName;
	std::string mCodeName;

	int32_t mSpawnFlag;
	int32_t mDespawnFlag;
	int32_t mEventNumber;
	int32_t mItemTableIndex;
	int32_t mSpawnPointID;
	int32_t mGBHScanID;

	EConditionType mCondType;
	EAttackType mAttackType;
	uint32_t mMoveType;
	EAppearType mAppearType;

	bool mIsVisible;
	bool mStay;

	std::weak_ptr<LItemAppearDOMNode> mItemTableRef;

public:
	typedef LEntityDOMNode Super;

	LCharacterDOMNode(std::string name);

	virtual std::string GetCreateName() const override { return mCreateName; }
	virtual void SetCreateName(std::string newCreateName) override { mCreateName = newCreateName; }

	virtual void RenderDetailsUI(float dt) override;

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

/*=== Type operations ===*/
	virtual const char* GetNodeTypeString() override { return "DOM_NODE_CHARACTER"; }

	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Character)
			return true;

		return Super::IsNodeType(type);
	}
};
