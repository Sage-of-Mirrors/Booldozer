#include "modes/EventMode.hpp"
#include "ResUtil.hpp"

LEventMode::LEventMode()
{

}

void LEventMode::RenderLeafContextMenu(std::shared_ptr<LDoorDOMNode> node)
{

}

void LEventMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Available Events", 0, window_flags);

	ImGui::Separator();
	
	auto events = current_map->GetChildrenOfType<LEventDataDOMNode>(EDOMNodeType::EventData);

	window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight() - 75.f), false, window_flags);

	for (uint32_t i = 0; i < events.size(); i++)
	{
		uint32_t selectionType = 0;

		ImGui::PushID(i);

		events[i]->RenderHierarchyUI(events[i], &mSelectionManager);
		
		//if (ImGui::BeginPopupContextItem())
		//	RenderLeafContextMenu(events[i]);


		//if (newNode != nullptr && newNode == doors[i])
		//	ImGui::SetScrollHereY(1.0f);

		ImGui::PopID();
	}

	ImGui::EndChild();
	ImGui::End();
}

void LEventMode::RenderDetailsWindow()
{
	ImGui::Begin("Event Script");

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr){
		std::shared_ptr<LEventDataDOMNode> selection = std::static_pointer_cast<LEventDataDOMNode>(mSelectionManager.GetPrimarySelection());

		if(mSelected != selection){
			if(mSelected != nullptr){
				mSelected->mEventScript = mEditor.GetText();
			}
			mEditor.SetText(selection->mEventScript);
			mSelected = selection;
		}
		selection->RenderDetailsUI(0, &mEditor);
	}

	ImGui::End();
}

void LEventMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{
	RenderSceneHierarchy(current_map);

	// Render the nodes so that we're sure new nodes are initialized.
	for (auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)) {
		node->RenderBG(0);
	}

	RenderDetailsWindow();
}

void LEventMode::OnBecomeActive()
{
	printf("Event mode switching in.\n");
	
}

void LEventMode::OnBecomeInactive()
{
	printf("Event mode switching out!\n");
}
