#pragma once

#include "EditorModeBase.hpp"

#include <imgui.h>

class LDoorMode : public LEditorModeBase
{
	void RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map);
	void RenderDetailsWindow();

public:
	LDoorMode();

	virtual void Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene) override;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() override;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() override;
};
#pragma once
