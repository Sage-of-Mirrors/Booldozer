#pragma once

#include "EntityDOMNode.hpp"

class LBlackoutDOMNode : public LEntityDOMNode
{
	bool bIsActiveDuringBlackout;

public:
	typedef LEntityDOMNode Super;

	LBlackoutDOMNode(std::string name, bool isBlackoutEntity);

	bool IsActiveDuringBlackout() { return bIsActiveDuringBlackout; }

	static bool FilterBlackoutEntities(std::shared_ptr<LBlackoutDOMNode> node, bool isBlackoutOnly)
	{
		return node->bIsActiveDuringBlackout == isBlackoutOnly;
	}

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Blackout)
			return true;

		return Super::IsNodeType(type);
	}
};