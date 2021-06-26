#pragma once

#include "DOM.hpp"
#include "EditorSelection.hpp"
#include "history/EditorHistory.hpp"

class LEditorModeBase
{
protected:
	// The object that manages this mode's node selection.
	LEditorSelection mSelectionManager;
	// The object that manages this mode's undo/redo history.
	LEditorHistory mHistoryManager;

public:
	LEditorModeBase();

	virtual void Render(std::shared_ptr<LMapDOMNode> current_map/*, LEditorScene* renderer_scene*/) = 0;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() = 0;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() = 0;

	// Instructs the mode to attempt to perform an undo operation.
	virtual void Undo() { mHistoryManager.PerformUndo(); }
	// Instructs the mode to attempt to perform a redo operation.
	virtual void Redo() { mHistoryManager.PerformRedo(); }
};
