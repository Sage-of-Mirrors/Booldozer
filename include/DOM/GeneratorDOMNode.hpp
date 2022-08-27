#pragma once

#include "EntityDOMNode.hpp"

class LGeneratorDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mGenType;
	std::string mPathName;
	std::string mCodeName;

	int32_t mSpawnFlag;
	int32_t mDespawnFlag;

	int32_t mActorSpawnRate;
	int32_t mActorsPerBurst;
	int32_t mActorSpawnLimit;

	int32_t mArg0;
	int32_t mArg1;
	int32_t mArg2;
	int32_t mArg3;
	int32_t mArg4;
	int32_t mArg5;
	int32_t mArg6;
	int32_t mArg7;
	int32_t mArg8;

	bool mStay;

public:
	typedef LEntityDOMNode Super;

	LGeneratorDOMNode(std::string name);

	virtual std::string GetName() override { return mGenType; }

	virtual void RenderDetailsUI(float dt) override;

	void CopyTo(LGeneratorDOMNode* other);

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
		if (type == EDOMNodeType::Generator)
			return true;

		return Super::IsNodeType(type);
	}
};
