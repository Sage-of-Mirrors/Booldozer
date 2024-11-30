#pragma once

#include "EntityDOMNode.hpp"

class LBooDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	int32_t mInitialRoom;
	int32_t mNextRoomWait;

	float mAcceleration;
	float mMaxSpeed;
	float mAngle;

	int32_t mHP;
	int32_t mMoveTime;

	bool mAttacks;

public:
	typedef LEntityDOMNode Super;

	LBooDOMNode(std::string name);

	int32_t GetInitialRoom() { return mInitialRoom; }

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
		if (type == EDOMNodeType::Boo)
			return true;

		return Super::IsNodeType(type);
	}
};
