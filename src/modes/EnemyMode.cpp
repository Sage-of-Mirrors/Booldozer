#include "modes/EnemyMode.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

void LEnemyMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{

	ImGui::Begin("Scene Hierarchy");

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
	ImGui::Begin("Selected Object Details");

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LEnemyMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{
	LUIUtility::RenderGizmoToggle();
	RenderSceneHierarchy(current_map);
	RenderDetailsWindow();

	for (auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)) {
		if (!node->IsNodeType(EDOMNodeType::Enemy) && !node->IsNodeType(EDOMNodeType::Observer))
			continue;

		node->RenderBG(0, renderer_scene);
	}

	if (mSelectionManager.GetPrimarySelection() != nullptr) {
		glm::mat4* m = ((LBGRenderDOMNode*)(mSelectionManager.GetPrimarySelection().get()))->GetMat();
		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		ImGuizmo::Manipulate(&view[0][0], &proj[0][0], ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, &(*m)[0][0], NULL, NULL);
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
