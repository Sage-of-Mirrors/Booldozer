#include "modes/ItemMode.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

LItemMode::LItemMode()
{

}

void LItemMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGui::Begin("Scene Hierarchy");

	auto iteminfos = current_map->GetChildrenOfType<LItemInfoDOMNode>(EDOMNodeType::ItemInfo);
	if (ImGui::TreeNode("Master Item Table"))
	{
		for (uint32_t i = 0; i < iteminfos.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::Indent();

			ImGui::PushID(i);
			iteminfos[i]->RenderHierarchyUI(iteminfos[i], &mSelectionManager);
			ImGui::PopID();

			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	auto itemappears = current_map->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);
	if (ImGui::TreeNode("Item Drop Groups"))
	{
		for (uint32_t i = 0; i < itemappears.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::Indent();

			ImGui::PushID(i);
			itemappears[i]->RenderHierarchyUI(itemappears[i], &mSelectionManager);
			ImGui::PopID();

			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	auto itemfishings = current_map->GetChildrenOfType<LItemFishingDOMNode>(EDOMNodeType::ItemFishing);
	if (ImGui::TreeNode("Capture Item Groups"))
	{
		for (uint32_t i = 0; i < itemfishings.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::Indent();

			ImGui::PushID(i);
			itemfishings[i]->RenderHierarchyUI(itemfishings[i], &mSelectionManager);
			ImGui::PopID();

			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	auto trasuretables = current_map->GetChildrenOfType<LTreasureTableDOMNode>(EDOMNodeType::TreasureTable);
	if (ImGui::TreeNode("Treasure Tables"))
	{
		for (uint32_t i = 0; i < trasuretables.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::Indent();

			ImGui::PushID(i);
			trasuretables[i]->RenderHierarchyUI(trasuretables[i], &mSelectionManager);
			ImGui::PopID();

			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void LItemMode::RenderDetailsWindow()
{
	ImGui::Begin("Selected Object Details");

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LItemMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{
	RenderSceneHierarchy(current_map);
	RenderDetailsWindow();

	for (auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)) {
		node->RenderBG(0, renderer_scene);
	}
}

void LItemMode::OnBecomeActive()
{
	printf("Item mode switching in!\n");
}

void LItemMode::OnBecomeInactive()
{
	printf("Item mode switching out!\n");
}