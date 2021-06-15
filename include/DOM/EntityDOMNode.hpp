#pragma once

#include "BGRenderDOMNode.hpp"
#include "JmpIO.hpp"
#include "../lib/bStream/bstream.h"

class LEntityDOMNode : public LBGRenderDOMNode
{
public:
	typedef LBGRenderDOMNode Super;

	LEntityDOMNode(std::string name) : LBGRenderDOMNode(name) { mType = EDOMNodeType::Entity; }

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const = 0;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) = 0;

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Entity)
			return true;

		return Super::IsNodeType(type);
	}
};
