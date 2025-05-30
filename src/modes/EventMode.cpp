#include "modes/EventMode.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "imgui_internal.h"

LEventMode::LEventMode()
{
}

LEventMode::~LEventMode(){
}

void LEventMode::RenderLeafContextMenu(std::shared_ptr<LDoorDOMNode> node)
{
	
}

void LEventMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("sceneHierarchy", 0, window_flags);
	//ImGui::Text("Available Events");
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

	auto events = current_map->GetChildrenOfType<LEventDataDOMNode>(EDOMNodeType::EventData);

	window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowHeight() - 75.f), false, window_flags);

	for (uint32_t i = 0; i < events.size(); i++)
	{
		uint32_t selectionType = 0;

		ImGui::PushID(i);

		events[i]->RenderHierarchyUI(events[i], &mSelectionManager);

		ImGui::PopID();
	}

	ImGui::EndChild();
	ImGui::End();
}

void LEventMode::RenderDetailsWindow(LSceneCamera* camera)
{

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr){
		if(mSelectionManager.GetPrimarySelection()->GetNodeType() == EDOMNodeType::EventData){
			ImGui::Begin("toolWindow");

			std::shared_ptr<LEventDataDOMNode> selection = std::static_pointer_cast<LEventDataDOMNode>(mSelectionManager.GetPrimarySelection());

			if(mSelected != selection){
				if(mSelected != nullptr){
					mSelected->mEventScript = mEditorScript.GetText();
				}
				mEditorScript.SetText(selection->mEventScript);
				mSelected = selection;
			}

			ImGui::Text("Event");
			ImGui::SameLine();
			selection->RenderDetailsUI(0, &mEditorScript);

			ImGui::Separator();

			if(ImGui::Button("Save")){
				LGenUtility::Log << "[EventMode]: Saving Event Archive" << std::endl;
				selection->mEventScript = mEditorScript.GetText();
				selection->SaveEventArchive();
			}

			ImGui::End();
		} else {
			ImGui::Begin("toolWindow");
			ImGui::Text("Camera Animation");
			ImGui::Separator();
			std::static_pointer_cast<LCameraAnimationDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0, camera);
			ImGui::End();
		}
	}
}

void LEventMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode)
{

	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map, mode);

	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderDetailsWindow(&renderer_scene->Camera);

	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}

}

void LEventMode::RenderGizmo(LEditorScene* renderer_scene){

}

void LEventMode::OnBecomeActive()
{
	CameraAnimation::SetPreviewActive();
	LGenUtility::Log << "[Booldozer]: Event mode switching in" << std::endl;
}

void LEventMode::OnBecomeInactive()
{
	CameraAnimation::SetPreviewInactive();
	LGenUtility::Log << "[Booldozer]: Event mode switching out" << std::endl;
}
