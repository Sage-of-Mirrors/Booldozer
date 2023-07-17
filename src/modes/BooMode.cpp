#include "modes/BooMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

LBooMode::LBooMode()
{

}

void LBooMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGui::Begin("sceneHierarchy");
	ImGui::Text("Boo Data");
	ImGui::Separator();

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

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LBooMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
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


		ImGui::DockBuilderDockWindow("sceneHierarchy", mDockNodeUpLeftID);
		ImGui::DockBuilderDockWindow("detailWindow", mDockNodeDownLeftID);
		ImGui::DockBuilderDockWindow("toolWindow", mDockNodeRightID);

		ImGui::DockBuilderFinish(mMainDockSpaceID);
		bIsDockingSetUp = true;
	}

	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map);

	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderDetailsWindow();

	for (auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)) {
		node->RenderBG(0);
	}
}

void LBooMode::OnBecomeActive()
{
	printf("Boo mode switching in!\n");
}

void LBooMode::OnBecomeInactive()
{
	printf("Boo mode switching out!\n");
}
