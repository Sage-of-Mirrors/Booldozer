#pragma once

#include "EditorModeBase.hpp"

class LActorMode : public LEditorModeBase
{

	void RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map);
	void RenderDetailsWindow();
	std::weak_ptr<LRoomDOMNode> mManualRoomSelect;
	std::vector<std::shared_ptr<LEntityDOMNode>> mNodePrefabs;

	bool mRoomChanged { false };
	
public:
	LActorMode();

	virtual void Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene) override;
	virtual void RenderGizmo(LEditorScene* renderer_scene) override;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() override;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() override;
};
