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
	ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight() - 75.f), false, window_flags);

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
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingInCentralNode;
	mMainDockSpaceID = ImGui::DockSpaceOverViewport(mainViewport, dockFlags);
	
	if(!bIsDockingSetUp){
		ImGui::DockBuilderRemoveNode(mMainDockSpaceID); // clear any previous layout
		ImGui::DockBuilderAddNode(mMainDockSpaceID, dockFlags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(mMainDockSpaceID, mainViewport->Size);


		mDockNodeLeftID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Left, 0.25f, nullptr, &mMainDockSpaceID);
		mDockNodeRightID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Right, 0.25f, nullptr, &mMainDockSpaceID);
		mDockNodeDownLeftID = ImGui::DockBuilderSplitNode(mDockNodeLeftID, ImGuiDir_Down, 0.5f, nullptr, &mDockNodeUpLeftID);
		mDockNodeBottom = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Down, 0.25f, nullptr, &mMainDockSpaceID);


		ImGui::DockBuilderDockWindow("sceneHierarchy", mDockNodeUpLeftID);
		ImGui::DockBuilderDockWindow("detailWindow", mDockNodeDownLeftID);
		ImGui::DockBuilderDockWindow("toolWindow", mDockNodeRightID);
		ImGui::DockBuilderDockWindow("bottomPanel", mDockNodeBottom);

		ImGui::DockBuilderFinish(mMainDockSpaceID);
		bIsDockingSetUp = true;
	}

	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map);

	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderDetailsWindow(&renderer_scene->Camera);

	// Render the nodes so that we're sure new nodes are initialized.
	for (auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)) {
		node->RenderBG(0);
	}
}

void LEventMode::OnBecomeActive()
{
	printf("Event mode switching in.\n");
	
}

void LEventMode::OnBecomeInactive()
{
	printf("Event mode switching out!\n");
}
