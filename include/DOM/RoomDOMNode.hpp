#pragma once

#include "BGRenderDOMNode.hpp"
#include "JmpIO.hpp"
#include "Model.hpp"
#include "../lib/libgctools/include/archive.h"
#include "glm/glm.hpp"

// DOM node representing a single room, including its model and all of the objects within it.
class LRoomDOMNode : public LBGRenderDOMNode
{
	LModel mRoomModel;
	std::vector<std::shared_ptr<LModel>> mFurnitureModels;

/*=== Roominfo properties ===*/
	// Room's name in the JMP file. Usually just the string "room".
	std::string mInternalName;
	// This room's ID.
	int32_t mRoomNumber;
	// ??
	int32_t mThunder;
	// Whether the map's skybox should be rendered while the player is in this room.
	bool mShouldRenderSkybox;
	// ??
	int32_t mDustLevel;
	// The color of the lights in the room?
	glm::ivec3 mLightColor;
	// ??
	int32_t mDistance;
	// ??
	int32_t mLv;
	// ??
	int32_t mSoundEchoParameter;
	// ??
	int32_t mSoundRoomCode;
	// ??
	uint32_t mSoundRoomSize;

/*=== map.dat properties ===*/
	// TODO

	std::vector<std::string> mCompletedWaves;

public:
	typedef LBGRenderDOMNode Super;

	LRoomDOMNode(std::string name);

	std::string GetName() { return mName; }
	int32_t GetRoomNumber() { return mRoomNumber; }

	// Reads the data from the specified entry in the given LJmpIO instance into this room's JMP properties.
	void LoadJmpInfo(uint32_t index, LJmpIO* jmp_io);
	// Writes the JMP data from this room into the given LJmpIO instance at the specified entry.
	void SaveJmpInfo(uint32_t index, LJmpIO* jmp_io);
	// Loads the BIN models from the given archive, distributes them to entities that need them, and does various other room-specific loading stuff.
	bool CompleteLoad(GCarchive* room_arc);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Room)
			return true;

		return Super::IsNodeType(type);
	}
};
