#pragma once

#include "EntityDOMNode.hpp"

enum class LConditionType : uint32_t
{
	Always_True,
	Find_Player_On_Path,
	Always_True_2,
	All_Enemies_Dead,
	Check_GBH_Timer,
	Always_True_3,
	Find_Player_Far,
	Find_Player_Near,
	Find_Player_Infinite,
	All_Candles_Lit,
	All_Candles_Extinguished,
	Flag_91_Set,
	All_Water_Generators_Off,
	Room_Lights_On,
	Enemy_Group_Dead,
	In_Same_Room,
	Room_Lights_Off,
	Access_Name_Valid,
	Flag_Arg_Set,
	Flag_Arg_Not_Set
};

enum class LDoType : uint32_t
{
	Nothing,
	Turn_Room_Lights_On,
	Nothing_2,
	Telephone,
	Path_Loop_Start,
	Path_Loop_Middle,
	Path_Loop_End,
	Set_Flag_Arg,
	Chandelier_Fall_UNUSED,
	Play_Sound_or_Anim,
	Spawn_Create_Name_Entities,
	Lock_All_Doors,
	Unlock_All_Doors
};

class LObserverDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mCodeName;
	std::string mCondStringArg0;
	std::string mStringArg0;

	int32_t mCondArg0;
	int32_t mArg0;
	int32_t mArg1;
	int32_t mArg2;
	int32_t mArg3;
	int32_t mArg4;
	int32_t mArg5;
	int32_t mSpawnFlag;
	int32_t mDespawnFlag;

	LConditionType mCondType;
	LDoType mDoType;

	bool mIsVisible;
	bool mUnkBool1;

public:
	typedef LEntityDOMNode Super;

	LObserverDOMNode(std::string name);

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	std::string GetStringArg() { return mStringArg0; }
	LConditionType GetConditionType() { return mCondType; }
	LDoType GetDoType() { return mDoType; }

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Observer)
			return true;

		return Super::IsNodeType(type);
	}
};