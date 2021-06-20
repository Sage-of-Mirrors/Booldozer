#pragma once

#include "EntityDOMNode.hpp"

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
	
	uint32_t mEventIf;

	bool mCanBeInterrupted;
	bool mFreezePlayer;

public:
	typedef LEntityDOMNode Super;

	LEventDOMNode(std::string name);

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;
};