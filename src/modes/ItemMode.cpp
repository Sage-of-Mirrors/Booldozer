#include "modes/ItemMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

LItemMode::LItemMode()
{

}

void LItemMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGui::Begin("sceneHierarchy");
	ImGui::Text("Item Data");
	ImGui::Separator();

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

void LItemMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
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

void LItemMode::OnBecomeActive()
{
	printf("Item mode switching in!\n");
}

void LItemMode::OnBecomeInactive()
{
	printf("Item mode switching out!\n");
}
