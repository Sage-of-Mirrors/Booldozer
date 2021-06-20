#pragma once

#include "EntityDOMNode.hpp"

class LObserverDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mInternalName;
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

	uint32_t mCondType;
	uint32_t mDoType;

	bool mIsVisible;
	bool mUnkBool1;

public:
	typedef LEntityDOMNode Super;

	LObserverDOMNode(std::string name);

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;
};