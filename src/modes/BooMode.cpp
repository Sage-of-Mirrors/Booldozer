#include "modes/BooMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

LBooMode::LBooMode()
{

}

void LBooMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode)
{
	ImGui::Begin("sceneHierarchy");
	//ImGui::Text("Boo Data");
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

	auto boos = current_map->GetChildrenOfType<LBooDOMNode>(EDOMNodeType::Boo);

	for (std::shared_ptr<LBooDOMNode> boo : boos){
		boo->RenderHierarchyUI(boo, &mSelectionManager);
	}
	

	ImGui::End();
}

void LBooMode::RenderDetailsWindow()
{
	ImGui::Begin("detailWindow");
	ImGui::Text("Boo Details");
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

void LBooMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode)
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

	mPreviousSelection = mSelectionManager.GetPrimarySelection();
}

void LBooMode::RenderGizmo(LEditorScene* renderer_scene){
	if(mSelectionManager.GetPrimarySelection() != nullptr){
		if(!mSelectionManager.GetPrimarySelection()->IsNodeType(EDOMNodeType::Boo)){
			mSelectionManager.ClearSelection();
		} else {
			if(mPreviousSelection == nullptr || mPreviousSelection != mSelectionManager.GetPrimarySelection()){
				uint32_t roomNumber = std::dynamic_pointer_cast<LBooDOMNode>(mPreviousSelection)->GetInitialRoom();
				if(!renderer_scene->HasRoomLoaded(roomNumber)){
					LGenUtility::Log << "[Boo Mode]: Room Number trying to load " << roomNumber << std::endl;
					renderer_scene->SetRoom(mPreviousSelection->GetParentOfType<LMapDOMNode>(EDOMNodeType::Map).lock()->GetRoomByNumber(roomNumber));
				}
			}
		}
	}
}

void LBooMode::OnBecomeActive()
{
	LGenUtility::Log << "[Booldozer]: Boo mode switching in!\n" << std::endl;
}

void LBooMode::OnBecomeInactive()
{
	LGenUtility::Log << "[Booldozer]: Boo mode switching out!\n" << std::endl;
}
