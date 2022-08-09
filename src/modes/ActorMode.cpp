#include "modes/ActorMode.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

LActorMode::LActorMode()
{
	mRoomChanged = false;
}

void LActorMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGui::Begin("Scene Hierarchy");

	mRoomChanged = LUIUtility::RenderNodeReferenceCombo("Room to Render", EDOMNodeType::Room, current_map, mManualRoomSelect);

	auto rooms = current_map->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	for (uint32_t i = 0; i < rooms.size(); i++)
	{
		ImGui::PushID(i);
		rooms[i]->RenderHierarchyUI(rooms[i], &mSelectionManager);
		ImGui::PopID();
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

	//render prefabs
	ImGui::Begin("Prefab Nodes");

	if(ImGui::TreeNode("Nodes")){
		if(ImGui::BeginDragDropTarget()){
			LDOMNodeBase* dragDropNode = nullptr;

			const ImGuiPayload* payload = ImGui::GetDragDropPayload();

			if (payload != nullptr && payload->Data != nullptr)
			{
				if (ImGui::AcceptDragDropPayload(payload->DataType) != nullptr) dragDropNode = *(LEntityDOMNode**)payload->Data;
			}

			// Skip if there's no pending drag and drop to handle
			if (dragDropNode != nullptr)
			{
				std::shared_ptr<LEntityDOMNode> sharedNode = dragDropNode->GetSharedPtr<LEntityDOMNode>(EDOMNodeType::Entity);

				//if(mNodePrefabs.count(sharedNode) == 0){ //make sure we only add it once
					mNodePrefabs.push_back(sharedNode);
				//}
			}
			ImGui::EndDragDropTarget();
		}

		for (auto node : mNodePrefabs)
		{
			std::static_pointer_cast<LUIRenderDOMNode>(node)->RenderHierarchyUI(node, &mSelectionManager);
		}
		

		ImGui::TreePop();
	}

	ImGui::End();


	//LUIUtility::RenderGizmoToggle();
	RenderSceneHierarchy(current_map);
	RenderDetailsWindow();
	
	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}

	if(mSelectionManager.GetPrimarySelection() != nullptr){
		
		if(mPreviousSelection == nullptr || mPreviousSelection != mSelectionManager.GetPrimarySelection()){
			mPreviousSelection = mSelectionManager.GetPrimarySelection();
			if(!mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).expired() && !renderer_scene->HasRoomLoaded(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock()->GetRoomNumber())){
				renderer_scene->SetRoom(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock());				
				mManualRoomSelect = mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room);
			}
		}

		glm::mat4* m = ((LBGRenderDOMNode*)(mSelectionManager.GetPrimarySelection().get()))->GetMat();
		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::LOCAL, &(*m)[0][0], NULL, NULL);
	}

	if(mRoomChanged){
		renderer_scene->SetRoom(mManualRoomSelect.lock());
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
