#pragma once

#include "BGRenderDOMNode.hpp"

class LRoomDOMNode : LBGRenderDOMNode
{
public:
	typedef LBGRenderDOMNode Super;

	LRoomDOMNode(std::string name);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Room)
			return true;

		return Super::IsNodeType(type);
	}
};
