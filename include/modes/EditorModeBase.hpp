#pragma once

#include "DOM.hpp"
#include "EditorSelection.hpp"
#include "scene/EditorScene.hpp"
#include "history/EditorHistory.hpp"
#include "ImGuizmo.h"

enum class EEditorMode : uint32_t
{
	Actor_Mode,
	Enemy_Mode,
	Door_Mode,
	Path_Mode,
	Item_Mode,
	Event_Mode,
	Boo_Mode
};

class LEditorModeBase
{
protected:
	std::shared_ptr<LDOMNodeBase> mPreviousSelection;

	// The object that manages this mode's node selection.
	LEditorSelection mSelectionManager;
	// The object that manages this mode's undo/redo history.
	LEditorHistory mHistoryManager;

public:

	// The Gizmo's current operation mode (translate, rotate, scale)
	ImGuizmo::OPERATION mGizmoMode { ImGuizmo::OPERATION::TRANSLATE };
	
	virtual void Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode) = 0;

	virtual void RenderGizmo(LEditorScene* renderer_scene) = 0;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() = 0;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() = 0;

	LEditorSelection* GetSelectionManager() { return &mSelectionManager; }

	// Instructs the mode to attempt to perform an undo operation.
	virtual void Undo() { mHistoryManager.PerformUndo(); }
	// Instructs the mode to attempt to perform a redo operation.
	virtual void Redo() { mHistoryManager.PerformRedo(); }
};
