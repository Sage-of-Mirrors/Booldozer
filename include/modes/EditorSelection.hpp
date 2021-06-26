#pragma once

#include "DOM/DOMNodeBase.hpp"

// Manager for node selection within the various modes.
class LEditorSelection
{
	std::vector<std::shared_ptr<LDOMNodeBase>> mCurrentSelection;

public:
	LEditorSelection() { }

	// Adds the given node to the current selection, if not already present.
	void AddToSelection(std::shared_ptr<LDOMNodeBase> node);
	// Removes the given node from the current selection, if present.
	void RemoveFromSelection(std::shared_ptr<LDOMNodeBase> node);

	// Returns the first selected node in the current selection, or nullptr if the selection is currently empty.
	std::shared_ptr<LDOMNodeBase> GetPrimarySelection() { return mCurrentSelection.size() >= 1 ? mCurrentSelection[0] : nullptr; }
	// Returns the full collection of selected nodes.
	std::vector<std::shared_ptr<LDOMNodeBase>> GetSelection() { return mCurrentSelection; }

	// Removes all currently selected nodes.
	void ClearSelection() { mCurrentSelection.clear(); }

	// Returns whether there are currently 0 nodes selected.
	bool IsEmpty() { return mCurrentSelection.size() == 0; }
	// Returns whether there is currently exactly 1 node selected.
	bool IsSingleSelection() { return mCurrentSelection.size() == 1; }
	// Returns whether there are currently multiple nodes selected.
	bool IsMultiSelection() { return mCurrentSelection.size() > 1; }
};
