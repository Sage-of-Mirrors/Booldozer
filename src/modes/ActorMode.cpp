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

	auto objects = current_map->GetChildrenOfType<LObjectDOMNode>(EDOMNodeType::Object);
	if (ImGui::TreeNode("Objects"))
	{
		for (uint32_t i = 0; i < objects.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			objects[i]->RenderHierarchyUI(objects[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto keys = current_map->GetChildrenOfType<LKeyDOMNode>(EDOMNodeType::Key);
	if (ImGui::TreeNode("Keys"))
	{
		for (uint32_t i = 0; i < keys.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			keys[i]->RenderHierarchyUI(keys[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto speedys = current_map->GetChildrenOfType<LSpeedySpiritDropDOMNode>(EDOMNodeType::SpeedySpiritDrop);
	if (ImGui::TreeNode("Speedy Spirit Drops"))
	{
		for (uint32_t i = 0; i < speedys.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			speedys[i]->RenderHierarchyUI(speedys[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto boos = current_map->GetChildrenOfType<LBooDOMNode>(EDOMNodeType::Boo);
	if (ImGui::TreeNode("Boos"))
	{
		for (uint32_t i = 0; i < boos.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			boos[i]->RenderHierarchyUI(boos[i], &mSelectionManager);
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	auto doors = current_map->GetChildrenOfType<LDoorDOMNode>(EDOMNodeType::Door);
	if (ImGui::TreeNode("Doors"))
	{
		for (uint32_t i = 0; i < doors.size(); i++)
		{
			uint32_t selectionType = 0;

			ImGui::PushID(i);
			doors[i]->RenderHierarchyUI(doors[i], &mSelectionManager);
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
	//LUIUtility::RenderGizmoToggle();
	RenderSceneHierarchy(current_map);
	RenderDetailsWindow();
	
	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}

	if(mSelectionManager.GetPrimarySelection() != nullptr){
		
		if(mPreviousSelection == nullptr || mPreviousSelection != mSelectionManager.GetPrimarySelection()){
			mPreviousSelection = mSelectionManager.GetPrimarySelection();
			if(!renderer_scene->HasRoomLoaded(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock()->GetRoomNumber())){
				renderer_scene->SetRoom(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock());				
			}
		}

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
