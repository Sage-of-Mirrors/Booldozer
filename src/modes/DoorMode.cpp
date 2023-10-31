#include "modes/DoorMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"

LDoorMode::LDoorMode()
{

}

void LDoorMode::RenderLeafContextMenu(std::shared_ptr<LDoorDOMNode> node)
{
	if (ImGui::Selectable("Delete"))
	{
		auto mapNode = node->GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
		if (auto mapNodeLocked = mapNode.lock())
		{
			for (auto selected : mSelectionManager.GetSelection())
			{
				mapNodeLocked->RemoveChild(selected);
			}

			mSelectionManager.ClearSelection();
		}
	}

	ImGui::EndPopup();
}

void LDoorMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("sceneHierarchy", 0, window_flags);
	ImGui::Text("Doors");
	ImGui::Separator();

	auto doors = current_map->GetChildrenOfType<LDoorDOMNode>(EDOMNodeType::Door);

	std::shared_ptr<LDoorDOMNode> newNode = nullptr;
	if (ImGui::Button("Add Door"))
	{
		newNode = std::make_shared<LDoorDOMNode>("new door");
		newNode->AssignJmpIdAndIndex(doors);
		current_map->AddChild(newNode);
		//mSelectionManager.AddToSelection(newNode);
	}

	ImGui::Separator();

	window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight() - 75.f), false, window_flags);

	for (uint32_t i = 0; i < doors.size(); i++)
	{
		uint32_t selectionType = 0;

		ImGui::PushID(i);

		doors[i]->RenderHierarchyUI(doors[i], &mSelectionManager);
		if (ImGui::BeginPopupContextItem())
			RenderLeafContextMenu(doors[i]);

		if (newNode != nullptr && newNode == doors[i])
			ImGui::SetScrollHereY(1.0f);

		ImGui::PopID();
	}

	ImGui::EndChild();
	ImGui::End();
}

void LDoorMode::RenderDetailsWindow()
{
	ImGui::Begin("detailWindow");

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LDoorMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{
	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map);

	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderDetailsWindow();

	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}

}

void LDoorMode::RenderGizmo(LEditorScene* renderer_scene){
	if (mSelectionManager.GetPrimarySelection() != nullptr)
	{
		glm::mat4* m = ((LBGRenderDOMNode*)(mSelectionManager.GetPrimarySelection().get()))->GetMat();
		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::WORLD, &(*m)[0][0], NULL, NULL);
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
