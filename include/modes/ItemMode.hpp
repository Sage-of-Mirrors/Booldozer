#pragma once

#include "EditorModeBase.hpp"

#include <imgui.h>

class LItemInfoDOMNode;
class LItemAppearDOMNode;
class LItemFishingDOMNode;

class LItemMode : public LEditorModeBase
{
	void RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode);
	void RenderDetailsWindow();

	template<typename T>
	void RenderLeafContextMenu(std::shared_ptr<T> infoNode)
	{
		if (ImGui::Selectable("Delete"))
		{
			auto mapNode = infoNode->template GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
			if (auto mapNodeLocked = mapNode.lock())
			{
				mapNodeLocked->RemoveChild(infoNode);
				if (mSelectionManager.GetPrimarySelection() == infoNode)
					mSelectionManager.ClearSelection();
			}
		}

		ImGui::EndPopup();
	}

public:
	LItemMode();

	virtual void Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode) override;
	virtual void RenderGizmo(LEditorScene* renderer_scene) override;

	// Called when this mode becomes the active (currently interactable) mode.
	virtual void OnBecomeActive() override;
	// Called when this mode becomes inactive.
	virtual void OnBecomeInactive() override;
};
