#pragma once

#include "EditorModeBase.hpp"
#include "EditorSelection.hpp"

class LPathMode : public LEditorModeBase
{
	void RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode);
	void RenderDetailsWindow();

	bool bLastClickedWasPoint { false };
	LEditorSelection mPointSelection;

	bool RenderPointContextMenu(std::shared_ptr<LPathDOMNode> path, std::shared_ptr<LPathPointDOMNode> point);
	bool RenderPathContextMenu(std::shared_ptr<LPathDOMNode> path);
	void RenderRoomContextMenu(std::shared_ptr<LRoomDOMNode> room);

	LPathDOMNode* GetPathDragDropNode();

public:
	LPathMode();

	virtual void Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode) override;
	virtual void RenderGizmo(LEditorScene* renderer_scene) override;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() override;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() override;
};
