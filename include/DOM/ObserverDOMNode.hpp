#pragma once

#include "EntityDOMNode.hpp"

enum class EConditionType : uint32_t
{
	Always_True,
	Find_Player_along_Path,
	Always_True_2,
	All_Enemies_are_Dead,
	Check_GBH_Time,
	Always_True_3,
	Find_Player_at_Long_Range,
	Find_Player_at_Close_Range,
	Find_Player_at_Infinite_Range,
	All_Candles_are_Lit,
	All_Candles_are_Extinguished,
	Flag_91_is_Set,
	All_Water_Generators_are_Off,
	Room_Lights_are_On,
	Spawn_Group_is_Dead,
	Object_is_in_Same_Room_as_Player,
	Room_Lights_are_Off,
	Access_Name_is_Valid,
	Flag_Arg_is_Set,
	Flag_Arg_is_Not_Set
};

enum class EDoType : uint32_t
{
	Nothing,
	Turn_Room_Lights_On,
	Nothing_2,
	Telephone,
	Path_Loop_Start,
	Path_Loop_Middle,
	Path_Loop_End,
	Set_Flag_Arg,
	UNUSED_Chandelier_Fall,
	Play_Sound_or_Anim,
	Trigger_Spawn_Group,
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

	EConditionType mCondType;
	EDoType mDoType;

	bool mIsVisible;
	bool mUnkBool1;

public:
	typedef LEntityDOMNode Super;

	LObserverDOMNode(std::string name);

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

	std::string GetStringArg() { return mStringArg0; }
	EConditionType GetConditionType() { return mCondType; }
	EDoType GetDoType() { return mDoType; }

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Observer)
			return true;

		return Super::IsNodeType(type);
	}
};