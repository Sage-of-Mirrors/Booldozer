#include "modes/EnemyMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

void LEnemyMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode)
{

	ImGui::Begin("sceneHierarchy");
	//ImGui::Text("Rooms");
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

	auto rooms = current_map->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	for (uint32_t i = 0; i < rooms.size(); i++)
	{
		uint32_t selectionType = 0;

		ImGui::PushID(i);
		rooms[i]->RenderWaveHierarchyUI(rooms[i], &mSelectionManager);
		ImGui::PopID();
	}

	ImGui::End();
}

void LEnemyMode::RenderDetailsWindow()
{
	ImGui::Begin("detailWindow");
	ImGui::Text("Selected Object Details");
	ImGui::Separator();

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

void LEnemyMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode)
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

void LEnemyMode::RenderGizmo(LEditorScene* renderer_scene){
	if (mSelectionManager.GetPrimarySelection() != nullptr) {
		glm::mat4* m = ((LBGRenderDOMNode*)(mSelectionManager.GetPrimarySelection().get()))->GetMat();
		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::LOCAL, &(*m)[0][0], NULL, NULL);
		
		if(!mSelectionManager.GetPrimarySelection()->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).expired() && !renderer_scene->HasRoomLoaded(mSelectionManager.GetPrimarySelection()->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock()->GetRoomNumber())){
			renderer_scene->SetRoom(mSelectionManager.GetPrimarySelection()->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock());
		}
	}
	mPreviousSelection = mSelectionManager.GetPrimarySelection();
}

void LEnemyMode::OnBecomeActive()
{
	LGenUtility::Log << "[Booldozer]: Enemy mode switching in" << std::endl;
}

void LEnemyMode::OnBecomeInactive()
{
	LGenUtility::Log << "[Booldozer]: Enemy mode switching out" << std::endl;
}
