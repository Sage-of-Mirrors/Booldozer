#pragma once

#include "EditorModeBase.hpp"

extern bool isRoomDirty;

class LActorMode : public LEditorModeBase
{

	void RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode);
	void RenderDetailsWindow();
	std::weak_ptr<LRoomDOMNode> mManualRoomSelect;

	bool mRoomChanged { false };
	bool mGizmoWasUsing { false };
	bool mOriginalRoomBoundMin { false };
	bool mRoomBoundEdited { false };
	glm::vec3 mGizmoTranslationDelta {0,0,0};
	glm::mat4 mOriginalTransform { 1.0f };

public:
	LActorMode();

	virtual void Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode) override;
	virtual void RenderGizmo(LEditorScene* renderer_scene) override;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() override;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() override;
};
