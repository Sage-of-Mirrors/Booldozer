#pragma once

#include "BlackoutDOMNode.hpp"
#include <map>

constexpr size_t ENEMY_TYPE_COUNT = 51;

enum class EConditionType : uint32_t;

class LFurnitureDOMNode;
class LItemAppearDOMNode;

enum class EAppearType : uint32_t
{
	Normal,
	UNUSED_WILL_CRASH,
	Normal_2,
	Unknown_1,
	Unknown_2,
	Unknown_3,
	Find_Partner_for_Ghost_Guys
};

enum class EPlaceType : uint32_t
{
	Always_Spawn_at_Initial_Position,
	Always_Spawn_at_Initial_Position_2,
	Spawn_along_Path
};

class LEnemyDOMNode : public LBlackoutDOMNode
{
/*=== JMP properties ===*/
	std::string mCreateName { "----" };
	std::string mPathName { "(null)" };
	std::string mAccessName { "(null)" };
	std::string mCodeName { "(null)" };

	int32_t mFloatingHeight { 0 };
	int32_t mAppearChance { 0 };

	int32_t mSpawnFlag { 0 };
	int32_t mDespawnFlag { 0 };

	int32_t mEventSetNumber { 0 };
	int32_t mItemTableIndex { 0 };

	EConditionType mCondType { 0 };
	uint32_t mMoveType { 0 };
	uint32_t mSearchType { 0 };
	EAppearType mAppearType { EAppearType::Normal };
	EPlaceType mPlaceType { EPlaceType::Always_Spawn_at_Initial_Position };
	
	bool mIsVisible { false };
	bool mStay { false };

/*=== Node references (converted to/from required reference data on Post-/PreProcess() ===*/
	// Reference to a furniture node, for hiding the enemy inside it.
	std::weak_ptr<LFurnitureDOMNode> mFurnitureNodeRef;
	// Reference to an item appear table, for spawning items when the furniture is interacted with.
	std::weak_ptr<LItemAppearDOMNode> mItemTableRef;

public:
	typedef LBlackoutDOMNode Super;

	LEnemyDOMNode(std::string name);
	LEnemyDOMNode(std::string name, bool isBlackoutEnemy);

	virtual std::string GetCreateName() const override { return mCreateName; }
	virtual void SetCreateName(std::string newCreateName) override { mCreateName = newCreateName; }

	virtual std::string GetName() override;

	virtual void RenderDetailsUI(float dt) override;

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

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