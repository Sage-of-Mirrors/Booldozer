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

void LDoorMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("sceneHierarchy", 0, window_flags);
	//ImGui::Text("Doors");
	//ImGui::Separator();
	
	if(ImGui::BeginTabBar("##modeTabs")){
		if(ImGui::BeginTabItem("Actors")){
			mode = EEditorMode::Actor_Mode;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Waves")){
			mode = EEditorMode::Enemy_Mode;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Doors")){
			mode = EEditorMode::Door_Mode;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Paths")){
			mode = EEditorMode::Path_Mode;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Items")){
			mode = EEditorMode::Item_Mode;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Events")){
			mode = EEditorMode::Event_Mode;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Boos")){
			mode = EEditorMode::Boo_Mode;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

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
	ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowHeight() - 75.f), false, window_flags);

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

	if(mPreviousSelection == mSelectionManager.GetPrimarySelection()){
		if (mSelectionManager.IsMultiSelection())
			ImGui::Text("[Multiple Selection]");
		else if (mSelectionManager.GetPrimarySelection() != nullptr)
			std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);
	} else if(mPreviousSelection != nullptr){
		std::static_pointer_cast<LUIRenderDOMNode>(mPreviousSelection)->RenderDetailsUI(0);
	}

	ImGui::End();
}

void LDoorMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode)
{
	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map, mode);

	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderDetailsWindow();

	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}

}

void LDoorMode::RenderGizmo(LEditorScene* renderer_scene){
	if (mSelectionManager.GetPrimarySelection() != nullptr)
	{

		if(!mSelectionManager.GetPrimarySelection()->IsNodeType(EDOMNodeType::Door)){
			mSelectionManager.ClearSelection();
		} else {
			glm::mat4* m = ((LBGRenderDOMNode*)(mSelectionManager.GetPrimarySelection().get()))->GetMat();
			glm::mat4 view = renderer_scene->getCameraView();
			glm::mat4 proj = renderer_scene->getCameraProj();
			ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::WORLD, &(*m)[0][0], NULL, NULL);
			
			auto rooms = std::dynamic_pointer_cast<LDoorDOMNode>(mSelectionManager.GetPrimarySelection())->GetRoomReferences();
			if(rooms.first != nullptr && rooms.second != nullptr && (!renderer_scene->HasRoomLoaded(rooms.first->GetRoomNumber())|| !renderer_scene->HasRoomLoaded(rooms.second->GetRoomNumber()))){
				renderer_scene->SetRoom(rooms.first);
			}
		}
	}
	mPreviousSelection = mSelectionManager.GetPrimarySelection();
}

void LDoorMode::OnBecomeActive()
{
	LGenUtility::Log << "[Booldozer]: Door mode switching in" << std::endl;
}

void LDoorMode::OnBecomeInactive()
{
	LGenUtility::Log << "[Booldozer]: Door mode switching out" << std::endl;
}
