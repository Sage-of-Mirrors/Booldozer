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

void LBooMode::RenderGizmo(LEditorScene* renderer_scene){

}

void LBooMode::OnBecomeActive()
{
	std::cout << "[Booldozer]: Boo mode switching in!\n" << std::endl;
}

void LBooMode::OnBecomeInactive()
{
	std::cout << "[Booldozer]: Boo mode switching out!\n" << std::endl;
}
