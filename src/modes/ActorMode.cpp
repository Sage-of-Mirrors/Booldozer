#pragma once

#include "modes/ActorMode.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

LActorMode::LActorMode()
{

}

void LActorMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGui::Begin("Scene Hierarchy");

	auto rooms = current_map->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	if (ImGui::TreeNode("Rooms"))
	{
		for (uint32_t i = 0; i < rooms.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			rooms[i]->RenderHierarchyUI(rooms[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto events = current_map->GetChildrenOfType<LEventDOMNode>(EDOMNodeType::Event);
	if (ImGui::TreeNode("Events"))
	{
		for (uint32_t i = 0; i < events.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			events[i]->RenderHierarchyUI(events[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto iteminfos = current_map->GetChildrenOfType<LItemInfoDOMNode>(EDOMNodeType::ItemInfo);
	if (ImGui::TreeNode("Item Info Table"))
	{
		for (uint32_t i = 0; i < iteminfos.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			iteminfos[i]->RenderHierarchyUI(iteminfos[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto itemappears = current_map->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);
	if (ImGui::TreeNode("Item Appear Table"))
	{
		for (uint32_t i = 0; i < itemappears.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			itemappears[i]->RenderHierarchyUI(itemappears[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto itemfishings = current_map->GetChildrenOfType<LItemFishingDOMNode>(EDOMNodeType::ItemFishing);
	if (ImGui::TreeNode("Item Vacuuming Table"))
	{
		for (uint32_t i = 0; i < itemfishings.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			itemfishings[i]->RenderHierarchyUI(itemfishings[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto trasuretables = current_map->GetChildrenOfType<LTreasureTableDOMNode>(EDOMNodeType::TreasureTable);
	if (ImGui::TreeNode("Treasure Tables"))
	{
		for (uint32_t i = 0; i < trasuretables.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			trasuretables[i]->RenderHierarchyUI(trasuretables[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void LActorMode::RenderDetailsWindow()
{
	ImGui::Begin("Selected Object Details");

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LActorMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{
	LUIUtility::RenderGizmoToggle();
	RenderSceneHierarchy(current_map);
	RenderDetailsWindow();
	
	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0, renderer_scene);
	}

	if(mSelectionManager.GetPrimarySelection() != nullptr){
		glm::mat4* m = ((LBGRenderDOMNode*)(mSelectionManager.GetPrimarySelection().get()))->GetMat();
		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		ImGuizmo::Manipulate(&view[0][0], &proj[0][0], ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, &(*m)[0][0], NULL, NULL);
	}

}

void LActorMode::OnBecomeActive()
{
	printf("Actor mode switching in!\n");
}

void LActorMode::OnBecomeInactive()
{
	printf("Actor mode switching out!\n");
}
