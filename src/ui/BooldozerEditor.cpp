#include "ui/BooldozerEditor.hpp"
#include "JmpIO.hpp"
#include "../lib/libgctools/include/compression.h"
#include "../lib/bStream/bstream.h"
#include "DOM/FurnitureDOMNode.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "imgui.h"

LBooldozerEditor::LBooldozerEditor()
{

}

void LBooldozerEditor::RenderSceneHierarchyUI(float dt)
{
	if (mLoadedMap.Children.empty())
		return;

	auto rooms = mLoadedMap.GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	if (ImGui::TreeNode("Rooms"))
	{
		for (uint32_t i = 0; i < rooms.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			rooms[i]->RenderHierarchyUI(dt);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto events = mLoadedMap.GetChildrenOfType<LEventDOMNode>(EDOMNodeType::Event);
	if (ImGui::TreeNode("Events"))
	{
		for (uint32_t i = 0; i < events.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			events[i]->RenderHierarchyUI(dt);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}
}

void LBooldozerEditor::onOpenMapCB()
{
	mLoadedMap.LoadMap(std::filesystem::path("D:\\SZS Tools\\Luigi's Mansion\\root\\files\\Map\\map2.szp"));
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "User selected open room(s)!" << std::endl;
}
