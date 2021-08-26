#pragma once

#include "BGRenderDOMNode.hpp"
#include "JmpIO.hpp"
#include "../lib/bStream/bstream.h"

enum LEntityType
{
	LEntityType_Characters,
	LEntityType_Enemies,
	LEntityType_Events,
	LEntityType_Furniture,
	LEntityType_Generators,
	LEntityType_Groups,
	LEntityType_ItemAppear,
	LEntityType_ItemFishing,
	LEntityType_ItemInfoTable,
	LEntityType_ItemTable,
	LEntityType_SpeedySpiritDrops,
	LEntityType_Keys,
	LEntityType_Objects,
	LEntityType_Observers,
	LEntityType_Polygons,
	LEntityType_Paths,
	LEntityType_Rooms,
	LEntityType_SoundGroups,
	LEntityType_SoundPolygons,
	LEntityType_BlackoutCharacters,
	LEntityType_BlackoutEnemies,
	LEntityType_BlackoutKeys,
	LEntityType_BlackoutObservers,
	LEntityType_Boos,
	LEntityType_TreasureTable,
	LEntityType_Max,
};

class LEntityDOMNode : public LBGRenderDOMNode
{
protected:
	int32_t mRoomNumber;

public:
	typedef LBGRenderDOMNode Super;

	LEntityDOMNode(std::string name) : LBGRenderDOMNode(name) { mType = EDOMNodeType::Entity; mRoomNumber = -1; }

	int32_t GetRoomNumber() { return mRoomNumber; }
	virtual std::string GetCreateName() const { return "(null)"; }
	virtual void SetCreateName(std::string newCreateName) { };

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const = 0;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) = 0;

	// Fixes up things that need to be done post-load, like generating node reference members from indices or names.
	virtual void PostProcess() = 0;
	// Ensures that things that need to be done prior to saving are done, like converting from reference member to indices or names.
	virtual void PreProcess() = 0;

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Entity)
			return true;

		return Super::IsNodeType(type);
	}

	static EDOMNodeType EntityTypeToDOMNodeType(LEntityType t)
	{
		switch (t)
		{
		case LEntityType_Characters:
			return EDOMNodeType::Character;
		case LEntityType_BlackoutCharacters:
			return EDOMNodeType::BlackoutCharacter;

		case LEntityType_Enemies:
			return EDOMNodeType::Enemy;
		case LEntityType_BlackoutEnemies:
			return EDOMNodeType::BlackoutEnemy;

		case LEntityType_Keys:
			return EDOMNodeType::Key;
		case LEntityType_BlackoutKeys:
			return EDOMNodeType::BlackoutKey;

		case LEntityType_Observers:
			return EDOMNodeType::Observer;
		case LEntityType_BlackoutObservers:
			return EDOMNodeType::BlackoutObserver;

		case LEntityType_Events:
			return EDOMNodeType::Event;
		case LEntityType_Furniture:
			return EDOMNodeType::Furniture;
		case LEntityType_Generators:
			return EDOMNodeType::Generator;
		case LEntityType_Objects:
			return EDOMNodeType::Object;
		case LEntityType_Paths:
			return EDOMNodeType::Path;

		case LEntityType_ItemInfoTable:
			return EDOMNodeType::ItemInfo;
		case LEntityType_ItemAppear:
			return EDOMNodeType::ItemAppear;
		case LEntityType_ItemFishing:
			return EDOMNodeType::ItemFishing;
		case LEntityType_TreasureTable:
			return EDOMNodeType::TreasureTable;

		default:
			return EDOMNodeType::Map;
		}
	}
};
