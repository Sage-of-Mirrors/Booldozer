#include "modes/EditorSelection.hpp"
#include "imgui.h"
#include <algorithm>

void LEditorSelection::AddToSelection(std::shared_ptr<LDOMNodeBase> node)
{
	auto findResult = std::find(mCurrentSelection.begin(), mCurrentSelection.end(), node);
	bool ctrlPressed = ImGui::GetIO().KeyCtrl;

	if (findResult != mCurrentSelection.end() && ctrlPressed)
	{
		RemoveFromSelection(node);
		return;
	}
	
	if (!ctrlPressed)
	{
		for (auto p : mCurrentSelection)
		{
			p->SetIsSelected(false);
		}

		mCurrentSelection.clear();
	}

	node->SetIsSelected(true);
	mCurrentSelection.push_back(node);
}

void LEditorSelection::RemoveFromSelection(std::shared_ptr<LDOMNodeBase> node)
{
	auto findResult = std::find(mCurrentSelection.begin(), mCurrentSelection.end(), node);

	// The given node isn't in the selection, so don't do anything.
	if (findResult == mCurrentSelection.end())
		return;

	node->SetIsSelected(false);
	mCurrentSelection.erase(findResult);
}
