#pragma once

#include "EditorModeBase.hpp"

#include <imgui.h>
#include "TextEditor.h"

class LEventMode : public LEditorModeBase
{
	void RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map);
	void RenderDetailsWindow();

	TextEditor mEditor;
	std::shared_ptr<LEventDataDOMNode> mSelected;


public:
	LEventMode();

	void RenderLeafContextMenu(std::shared_ptr<LDoorDOMNode> node);

	virtual void Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene) override;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() override;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() override;
};
#pragma once