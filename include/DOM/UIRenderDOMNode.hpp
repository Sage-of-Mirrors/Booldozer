#pragma once

#include "DOMNodeBase.hpp"

// DOM node with the ability to render ImGui controls to a window.
class LUIRenderDOMNode : public LDOMNodeBase
{
public:
	typedef LDOMNodeBase Super;

	LUIRenderDOMNode(std::string name);

	// Renders this node to an ImGui window.
	virtual void RenderUI(float dt);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::UIRender)
			return true;

		return Super::IsNodeType(type);
	}
};
