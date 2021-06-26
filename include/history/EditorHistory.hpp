#pragma once

#include <memory>
#include <vector>

// Base struct for an undoable/redoable action.
struct LEditorHistoryItem
{
	virtual void Undo() = 0;
	virtual void Redo() = 0;
};

// Manages the history of an editor mode's actions.
class LEditorHistory
{
private:
	// Stack that contains items that can be undone. When popped, items go into mRedoStack.
	std::vector<std::shared_ptr<LEditorHistoryItem>> mUndoStack;
	// Stack that contains items that can be redone. When popped, items go into mUndoStack.
	std::vector<std::shared_ptr<LEditorHistoryItem>> mRedoStack;

public:
	LEditorHistory() { }

	// Adds the given history item to the undo stack. The redo stack is also emptied.
	void AddUndoItem(std::shared_ptr<LEditorHistoryItem> item);
	
	// Pops an item from the undo stack, performs its undo action, and pushes the item onto the redo stack.
	void PerformUndo();
	// Pops an item from the redo stack, performs its redo action, and pushes the item onto the undo stack.
	void PerformRedo();
};
