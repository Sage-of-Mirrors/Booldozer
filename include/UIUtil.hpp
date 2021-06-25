#pragma once

#include "imgui.h"
#include "DOM.hpp"

namespace LUIUtility
{
	// Renders a checkbox for the given boolean. Returns whether the checkbox was modified, in which case the bool pointer
	// now contains the new state.
	bool RenderCheckBox(std::string name, bool* c);
	// Renders a checkbox that toggles the given node's IsRendered state.
	void RenderCheckBox(LDOMNodeBase* node);

	// Renders an ImGui Selectable control and returns whether it was left-clicked.
	bool RenderNodeSelectable(LDOMNodeBase* node);
};
