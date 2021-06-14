#pragma once

#include "UIRenderDOMNode.hpp"

// DOM node with the ability to render models to the main window's background.
class LBGRenderDOMNode : public LUIRenderDOMNode
{
public:
	typedef LUIRenderDOMNode Super;

	LBGRenderDOMNode(std::string name);

	// Renders this node to the main window's background.
	virtual void RenderBG(float dt);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::BGRender)
			return true;

		return Super::IsNodeType(type);
	}
};
