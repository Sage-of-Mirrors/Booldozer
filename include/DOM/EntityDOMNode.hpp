#pragma once

#include "BGRenderDOMNode.hpp"
#include "../lib/libgctools/include/stream.h"

class LEntityDOMNode : public LBGRenderDOMNode
{
public:
	typedef LBGRenderDOMNode Super;

	LEntityDOMNode(std::string name) : LBGRenderDOMNode(name) { mType = EDOMNodeType::Entity; }

	virtual void Serialize(GCcontext* context, GCstream* reader) = 0;
	virtual void Deserialize(GCcontext* context, GCstream* reader) = 0;

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Entity)
			return true;

		return Super::IsNodeType(type);
	}
};