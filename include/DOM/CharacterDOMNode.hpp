#pragma once

#include "EntityDOMNode.hpp"

class LCharacterDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mCreateName;
	std::string mPathName;
	std::string mCodeName;

	int32_t mSpawnFlag;
	int32_t mDespawnFlag;
	int32_t mEventNumber;
	int32_t mItemTable;
	int32_t mSpawnPointID;
	int32_t mGBHScanID;

	uint32_t mCondType;
	uint32_t mAttackType;
	uint32_t mMoveType;
	uint32_t mAppearType;

	bool mIsVisible;
	bool mStay;

public:
	typedef LEntityDOMNode Super;

	LCharacterDOMNode(std::string name);

	virtual std::string GetCreateName() const override { return mCreateName; }
	virtual void SetCreateName(std::string newCreateName) override { mCreateName = newCreateName; }

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

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