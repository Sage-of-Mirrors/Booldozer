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
	LEntityType_TreasureChests,
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

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const = 0;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) = 0;

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Entity)
			return true;

		return Super::IsNodeType(type);
	}
};
