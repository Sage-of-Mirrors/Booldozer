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

void LEventMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("sceneHierarchy", 0, window_flags);
	ImGui::Text("Available Events");
	ImGui::Separator();
	
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
			ImGui::Text("Event Script");
			ImGui::Separator();
			std::shared_ptr<LEventDataDOMNode> selection = std::static_pointer_cast<LEventDataDOMNode>(mSelectionManager.GetPrimarySelection());

			if(ImGui::Button("Save Event")){
				std::cout << "[EventMode]: Saving Event Archive" << std::endl;
				selection->mEventScript = mEditor.GetText();
				selection->SaveEventArchive();
			}

			if(mSelected != selection){
				if(mSelected != nullptr){
					mSelected->mEventScript = mEditor.GetText();
				}
				mEditor.SetText(selection->mEventScript);
				mSelected = selection;
			}
			selection->RenderDetailsUI(0, &mEditor);
			ImGui::End();
		} else {
			ImGui::Begin("bottomPanel");
			ImGui::Text("Camera Animation");
			ImGui::Separator();
			std::static_pointer_cast<LCameraAnimationDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0, camera);
			ImGui::End();
		}
	}
}

void LEventMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{

	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map);

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
	std::cout << "[Booldozer]: Event mode switching in" << std::endl;
	
}

void LEventMode::OnBecomeInactive()
{
	std::cout << "[Booldozer]: Event mode switching out" << std::endl;
}
