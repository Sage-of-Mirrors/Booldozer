#pragma once

#include "EntityDOMNode.hpp"

constexpr int32_t HOUR_MAX = 23;
constexpr int32_t MINUTE_MAX = 59;

enum class EEventIfType : uint32_t
{
	Repeatedly_from_Anywhere,
	Press_A_in_Radius,
	In_Range,
	In_Range_On_Room_Enter,
	On_Enter_Range,
	Repeatedly_in_Range,
	In_Sphere
};

class LEventDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mCharacterName;

	int32_t mEventNo;
	int32_t mActivationRadius;
	int32_t mEventFlag;

	int32_t mMinHour;
	int32_t mMinMinute;
	int32_t mMaxHour;
	int32_t mMaxMinute;

	int32_t mMaxTriggerCount;
	int32_t mDespawnFlag;

	int32_t mParameter;
	
	EEventIfType mEventIf;

	bool mCanBeInterrupted;
	bool mFreezePlayer;

public:
	typedef LEntityDOMNode Super;

	LEventDOMNode(std::string name);

	virtual void RenderDetailsUI(float dt) override;

	int32_t GetTriggerRadius() { return mActivationRadius; }

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Event)
			return true;

		return Super::IsNodeType(type);
	}
};