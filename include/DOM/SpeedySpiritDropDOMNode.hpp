#pragma once

#include "EntityDOMNode.hpp"

class LSpeedySpiritDropDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	int32_t mCoins;
	int32_t mBills;
	int32_t mGoldBars;
	int32_t mSapphires;
	int32_t mEmeralds;
	int32_t mRubies;

public:
	typedef LEntityDOMNode Super;

	LSpeedySpiritDropDOMNode(std::string name);

	virtual void RenderDetailsUI(float dt) override;

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
		if (type == EDOMNodeType::SpeedySpiritDrop)
			return true;

		return Super::IsNodeType(type);
	}
};
