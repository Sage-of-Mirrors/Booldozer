#pragma once

#include "EntityDOMNode.hpp"
#include <map>

constexpr size_t ENEMY_TYPE_COUNT = 51;

extern std::map<std::string, std::string> EnemyNames;

class LEnemyDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mCreateName;
	std::string mPathName;
	std::string mAccessName;
	std::string mCodeName;

	int32_t mFloatingHeight;
	int32_t mAppearChance;

	int32_t mSpawnFlag;
	int32_t mDespawnFlag;

	int32_t mEventNumber;
	int32_t mItemTable;

	uint32_t mCondType;
	uint32_t mMoveType;
	uint32_t mSearchType;
	uint32_t mAppearType;
	uint32_t mPlaceType;
	
	bool mIsVisible;
	bool mStay;

public:
	typedef LEntityDOMNode Super;

	LEnemyDOMNode(std::string name);

	virtual std::string GetCreateName() const override { return mCreateName; }
	virtual void SetCreateName(std::string newCreateName) override { mCreateName = newCreateName; }

	virtual std::string GetName() override { return EnemyNames[mName]; }
	virtual void RenderDetailsUI(float dt) override;

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

/*=== Type operations ===*/
	virtual const char* GetNodeTypeString() override { return "DOM_NODE_ENEMY"; }

	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Enemy)
			return true;

		return Super::IsNodeType(type);
	}
};