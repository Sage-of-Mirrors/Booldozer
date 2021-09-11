#pragma once

#include "EditorModeBase.hpp"

class LActorMode : public LEditorModeBase
{
	void RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map);
	void RenderDetailsWindow();
	std::weak_ptr<LRoomDOMNode> mManualRoomSelect;
	bool mRoomChanged;

public:
	LActorMode();

	virtual void Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene) override;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() override;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() override;
};
