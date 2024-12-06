#include "modes/ItemMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

LItemMode::LItemMode()
{

}

void LItemMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode)
{
	ImGui::Begin("sceneHierarchy");
	//ImGui::Text("Item Data");
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

	auto iteminfos = current_map->GetChildrenOfType<LItemInfoDOMNode>(EDOMNodeType::ItemInfo);
	bool definitionTreeOpened = ImGui::TreeNode("Item Definitions");
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Add Definition"))
		{
			auto newInfoNode = std::make_shared<LItemInfoDOMNode>("(null)");
			current_map->AddChild(newInfoNode);

			mSelectionManager.AddToSelection(newInfoNode);
		}

		ImGui::EndPopup();
	}

	if (definitionTreeOpened)
	{
		// We start at 1 because entry 0 is "nothing", which the game uses when there's no item to be spawned.
		// Going to prevent the user from modifying it because doing so could break the game.
		for (uint32_t i = 1; i < iteminfos.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::Indent();
			ImGui::PushID(i);

			iteminfos[i]->RenderHierarchyUI(iteminfos[i], &mSelectionManager);
			if (ImGui::BeginPopupContextItem())
				RenderLeafContextMenu(iteminfos[i]);

			ImGui::PopID();
			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	auto itemappears = current_map->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);
	bool lootOpened = ImGui::TreeNode("Loot Tables");
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Add Loot Table"))
		{
			auto newLootNode = std::make_shared<LItemAppearDOMNode>("(null)");
			current_map->AddChild(newLootNode);

			mSelectionManager.AddToSelection(newLootNode);
		}

		ImGui::EndPopup();
	}

	if (lootOpened)
	{
		for (uint32_t i = 0; i < itemappears.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::Indent();
			ImGui::PushID(i);

			itemappears[i]->RenderHierarchyUI(itemappears[i], &mSelectionManager);
			if (ImGui::BeginPopupContextItem())
				RenderLeafContextMenu(itemappears[i]);

			ImGui::PopID();
			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	auto itemfishings = current_map->GetChildrenOfType<LItemFishingDOMNode>(EDOMNodeType::ItemFishing);
	bool fishingOpened = ImGui::TreeNode("Fishing Tables");
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Add Fishing Table"))
		{
			auto newFishingNode = std::make_shared<LItemFishingDOMNode>("(null)");
			current_map->AddChild(newFishingNode);

			mSelectionManager.AddToSelection(newFishingNode);
		}

		ImGui::EndPopup();
	}

	if (fishingOpened)
	{
		for (uint32_t i = 0; i < itemfishings.size(); i++)
		{
			ImGui::Indent();
			ImGui::PushID(i);

			itemfishings[i]->RenderHierarchyUI(itemfishings[i], &mSelectionManager);
			if (ImGui::BeginPopupContextItem())
				RenderLeafContextMenu(itemfishings[i]);

			ImGui::PopID();
			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void LItemMode::RenderDetailsWindow()
{
	ImGui::Begin("detailWindow");
	ImGui::Text("Object Details");
	ImGui::Separator();

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LItemMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode)
{
	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map, mode);

	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderDetailsWindow();

	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}
}

void LItemMode::RenderGizmo(LEditorScene* renderer_scene){
}

void LItemMode::OnBecomeActive()
{
	LGenUtility::Log << "[Booldozer]: Item mode switching in" << std::endl;
}

void LItemMode::OnBecomeInactive()
{
	LGenUtility::Log << "[Booldozer]: Item mode switching out" << std::endl;
}
