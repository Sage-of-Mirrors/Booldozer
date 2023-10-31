#include "modes/EnemyMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

void LEnemyMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{

	ImGui::Begin("sceneHierarchy");
	ImGui::Text("Rooms");
	ImGui::Separator();

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

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LEnemyMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
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

void LEnemyMode::RenderGizmo(LEditorScene* renderer_scene){
	if (mSelectionManager.GetPrimarySelection() != nullptr) {
		glm::mat4* m = ((LBGRenderDOMNode*)(mSelectionManager.GetPrimarySelection().get()))->GetMat();
		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::LOCAL, &(*m)[0][0], NULL, NULL);
	}
}

void LEnemyMode::OnBecomeActive()
{
	printf("Enemy mode switching in!\n");
}

void LEnemyMode::OnBecomeInactive()
{
	printf("Enemy mode switching out!\n");
}
