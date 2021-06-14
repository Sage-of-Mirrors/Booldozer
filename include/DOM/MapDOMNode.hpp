#pragma once

#include "DOMNodeBase.hpp"

class LMapDOMNode : public LDOMNodeBase
{
public:
	typedef LDOMNodeBase Super;

	LMapDOMNode(std::string name);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Map)
			return true;

		return Super::IsNodeType(type);
	}
};
