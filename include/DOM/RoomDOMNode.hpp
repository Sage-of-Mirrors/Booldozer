#pragma once

#include "BGRenderDOMNode.hpp"
#include "EntityDOMNode.hpp"
#include "JmpIO.hpp"
#include "Model.hpp"
#include "../lib/libgctools/include/archive.h"
#include "glm/glm.hpp"
#include <memory>

class LObserverDOMNode;
class LEnemyDOMNode;

extern std::string const LRoomEntityTreeNodeNames[];

enum LRoomEntityType : uint32_t
{
	LRoomEntityType_Characters,
	LRoomEntityType_Enemies,
	LRoomEntityType_Furniture,
	LRoomEntityType_Generators,
	LRoomEntityType_Objects,
	LRoomEntityType_Observers,
	LRoomEntityType_Paths,
	LRoomEntityType_BlackoutCharacters,
	LRoomEntityType_BlackoutEnemies,
	LRoomEntityType_BlackoutObservers,
	LRoomEntityType_Max,
};

struct LSpawnGroup
{
	bool IsWaveCompleted = false;

	std::string CreateName;

	std::shared_ptr<LObserverDOMNode> ObserverNode;
	std::vector<std::shared_ptr<LEntityDOMNode>> EntityNodes;

	LSpawnGroup(std::string createName = "----", std::shared_ptr<LObserverDOMNode> observer = nullptr) { CreateName = createName; ObserverNode = observer; }

	std::string GetGroupName()
	{
		if (CreateName == "----")
			return "Default Group";
		
		return "Group " + CreateName;
	}
};

// DOM node representing a single room, including its model and all of the objects within it.
class LRoomDOMNode : public LBGRenderDOMNode
{
	std::vector<std::shared_ptr<LEntityDOMNode>> mRoomEntities[LRoomEntityType_Max];
	bool mRoomEntityVisibility[LRoomEntityType_Max] = {
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true
	};

	LRoomEntityType t = LRoomEntityType_Characters;

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

	std::vector<LSpawnGroup> Groups;

	void GetEntitiesWithCreateName(const std::string CreateName, const LRoomEntityType Type, std::vector<std::shared_ptr<LEntityDOMNode>>& TargetVec);
	LSpawnGroup* GetSpawnGroupWithCreateName(std::string createName);
	LEntityDOMNode* GetSpawnGroupDragDropNode();

public:
	typedef LBGRenderDOMNode Super;

	LRoomDOMNode(std::string name);

	int32_t GetRoomNumber() { return mRoomNumber; }

	virtual void RenderHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection) override;
	void RenderWaveHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection);

	// Reads the data from the specified entry in the given LJmpIO instance into this room's JMP properties.
	void LoadJmpInfo(uint32_t index, LJmpIO* jmp_io);
	// Writes the JMP data from this room into the given LJmpIO instance at the specified entry.
	void SaveJmpInfo(uint32_t index, LJmpIO* jmp_io);
	// Loads the BIN models from the given archive, distributes them to entities that need them, and does various other room-specific loading stuff.
	bool CompleteLoad(GCarchive* room_arc);

/*=== Type operations ===*/
	virtual const char* GetNodeTypeString() override { return "DOM_NODE_ROOM"; }

	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Room)
			return true;

		return Super::IsNodeType(type);
	}
};
