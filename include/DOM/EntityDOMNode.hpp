#pragma once

#include "BGRenderDOMNode.hpp"
#include "io/JmpIO.hpp"
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

class LEntityDOMNode : public LBGRenderDOMNode, public ISerializable
{
protected:
	int32_t mRoomNumber;

public:
	typedef LBGRenderDOMNode Super;

	LEntityDOMNode(std::string name) : LBGRenderDOMNode(name) { mType = EDOMNodeType::Entity; mRoomNumber = -1; }

	int32_t GetRoomNumber() { return mRoomNumber; }
	virtual std::string GetCreateName() const { return "(null)"; }
	virtual void SetCreateName(std::string newCreateName) { };

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
		case LEntityType_BlackoutCharacters:
			return EDOMNodeType::Character;

		case LEntityType_Enemies:
		case LEntityType_BlackoutEnemies:
			return EDOMNodeType::Enemy;

		case LEntityType_Keys:
		case LEntityType_BlackoutKeys:
			return EDOMNodeType::Key;

		case LEntityType_Observers:
		case LEntityType_BlackoutObservers:
			return EDOMNodeType::Observer;

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
		case LEntityType_SpeedySpiritDrops:
			return EDOMNodeType::SpeedySpiritDrop;
		case LEntityType_Boos:
			return EDOMNodeType::Boo;
		case LEntityType_Rooms:
			return EDOMNodeType::Room;
		default:
			return EDOMNodeType::Map;
		}
	}
};
