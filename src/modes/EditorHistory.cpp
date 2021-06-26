#include "history/EditorHistory.hpp"

void LEditorHistory::AddUndoItem(std::shared_ptr<LEditorHistoryItem> item)
{
	mUndoStack.push_back(item);
	
	// We have to clear the redo stack now in case there are any conflicting items.
	mRedoStack.clear();
}

void LEditorHistory::PerformUndo()
{
	if (mUndoStack.empty())
		return;

	auto undoItem = mUndoStack.back();
	undoItem->Undo();

	mRedoStack.push_back(undoItem);
	mUndoStack.pop_back();
}

void LEditorHistory::PerformRedo()
{
	if (mRedoStack.empty())
		return;

	auto redoItem = mRedoStack.back();
	redoItem->Redo();

	mUndoStack.push_back(redoItem);
	mRedoStack.pop_back();
}
