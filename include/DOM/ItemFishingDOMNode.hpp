#pragma once

#include "EntityDOMNode.hpp"

class LItemInfoDOMNode;

class LItemFishingDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	std::string mItemNames[60];

	std::weak_ptr<LItemInfoDOMNode> mItemInfoRefs[60];

public:
	typedef LEntityDOMNode Super;

	LItemFishingDOMNode(std::string name);

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
		if (type == EDOMNodeType::ItemFishing)
			return true;

		return Super::IsNodeType(type);
	}
};
