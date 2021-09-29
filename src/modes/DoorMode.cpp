#include "modes/DoorMode.hpp"

LDoorMode::LDoorMode()
{

}

void LDoorMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGui::Begin("Door List");

	auto doors = current_map->GetChildrenOfType<LDoorDOMNode>(EDOMNodeType::Door);
	bool definitionTreeOpened = ImGui::TreeNode("Doors");
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Add Door"))
		{
			auto newInfoNode = std::make_shared<LDoorDOMNode>("(null)");
			current_map->AddChild(newInfoNode);

			mSelectionManager.AddToSelection(newInfoNode);
		}

		ImGui::EndPopup();
	}

	if (definitionTreeOpened)
	{
		for (uint32_t i = 0; i < doors.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::Indent();
			ImGui::PushID(i);

			doors[i]->RenderHierarchyUI(doors[i], &mSelectionManager);
			//if (ImGui::BeginPopupContextItem())
				///RenderLeafContextMenu(iteminfos[i]);

			ImGui::PopID();
			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void LDoorMode::RenderDetailsWindow()
{
	ImGui::Begin("Selected Object Details");

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LDoorMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{
	RenderSceneHierarchy(current_map);
	RenderDetailsWindow();

	for (auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)) {
		node->RenderBG(0);
	}

	if (mSelectionManager.GetPrimarySelection() != nullptr)
	{
		glm::mat4* m = ((LBGRenderDOMNode*)(mSelectionManager.GetPrimarySelection().get()))->GetMat();
		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::LOCAL, &(*m)[0][0], NULL, NULL);
	}
}

void LDoorMode::OnBecomeActive()
{
	printf("Door mode switching in!\n");
}

void LDoorMode::OnBecomeInactive()
{
	printf("Door mode switching out!\n");
}
