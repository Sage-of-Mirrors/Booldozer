#include "modes/EditorSelection.hpp"
#include "imgui.h"

void LEditorSelection::AddToSelection(std::shared_ptr<LDOMNodeBase> node)
{
	// TODO: Check for ctrl key so we can add/remove bulk

	auto findResult = std::find(mCurrentSelection.begin(), mCurrentSelection.end(), node);

	// The given node is already in the selection... so don't do anything.
	if (findResult != mCurrentSelection.end())
		return;

	node->SetIsSelected(true);
	mCurrentSelection.push_back(node);
}

void LEditorSelection::RemoveFromSelection(std::shared_ptr<LDOMNodeBase> node)
{
	// TODO: Check for ctrl key so we can add/remove bulk

	auto findResult = std::find(mCurrentSelection.begin(), mCurrentSelection.end(), node);

	// The given node isn't in the selection, so don't do anything.
	if (findResult == mCurrentSelection.end())
		return;

	node->SetIsSelected(false);
	mCurrentSelection.erase(findResult);
}
