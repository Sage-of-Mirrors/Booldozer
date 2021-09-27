#pragma once

#include "BGRenderDOMNode.hpp"
#include "EntityDOMNode.hpp"
#include "io/JmpIO.hpp"
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

struct LAlternateResource
{
	uint32_t mRoomNumber;
	uint32_t mUnknown1;
	uint32_t mUnknown2;
	uint32_t mUnknown3;

	std::string mAltResourceName;
};

struct LStaticRoomDefinition
{
	uint8_t mCameraPositionIndex;
	uint8_t mGBHFloor;
	uint8_t mDoorZone;
	uint8_t mRoomID;

	uint32_t mCameraBehavior;
	
	glm::vec3 mBoundingBoxMin;
	glm::vec3 mBoundingBoxMax;
	glm::vec3 mBoundingBoxCenter;

	uint32_t mUnknown1;
	uint32_t mUnknown2;

	//std::vector<std::weak_ptr<LDoorDOMNode>> mDoors;

	glm::vec4 mDarkLighting;
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
class LRoomDOMNode : public LBGRenderDOMNode, public ISerializable
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
	std::string mInternalName { "room" };
	// This room's ID.
	int32_t mRoomNumber { 0 };
	// ??
	int32_t mThunder { 0 };
	// Whether the map's skybox should be rendered while the player is in this room.
	bool mShouldRenderSkybox { false };
	// ??
	int32_t mDustLevel { 0 };
	// The color of the lights in the room?
	float mLightColor[3];
	// ??
	int32_t mDistance { 0 };
	// ??
	int32_t mLv { 0 };
	// ??
	int32_t mSoundEchoParameter { 0 };
	// ??
	int32_t mSoundRoomCode { 0 };
	// ??
	int32_t mSoundRoomSize { 0 };

/*=== map.dat properties ===*/
	// Name of the resource file, either a raw BIN or an archive.
	std::string mResourceName;
	// Which rooms should be loaded/visible from this room.
	std::vector<std::weak_ptr<LRoomDOMNode>> mAdjacentRooms;
	// Definition of an alternate resource to use for a room's main model. Only used for Guest Room (room_28)?
	LAlternateResource mAlternateResource;
	// This is the definition of the room from the DOL.
	LStaticRoomDefinition mStaticDefinition;

	std::vector<LSpawnGroup> Groups;

	void GetEntitiesWithCreateName(const std::string CreateName, const LRoomEntityType Type, std::vector<std::shared_ptr<LEntityDOMNode>>& TargetVec);
	LSpawnGroup* GetSpawnGroupWithCreateName(std::string createName);
	LEntityDOMNode* GetSpawnGroupDragDropNode();

public:
	typedef LBGRenderDOMNode Super;

	LRoomDOMNode(std::string name);

	int32_t GetRoomNumber() const { return mRoomNumber; }
	void SetRoomNumber(int32_t roomNumber) { mRoomNumber = roomNumber; }

	virtual void RenderDetailsUI(float dt) override;
	virtual void RenderHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection) override;
	void RenderWaveHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection);

	// Reads the data from the specified entry in the given LJmpIO instance into this room's JMP properties.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;
	// Writes the JMP data from this room into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;

	virtual void PostProcess() override { };
	virtual void PreProcess() override { };

	// Loads the BIN models from the given archive, distributes them to entities that need them, and does various other room-specific loading stuff.
	bool CompleteLoad();

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
