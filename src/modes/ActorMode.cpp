#include "modes/ActorMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

//#include <DiscordIntegration.hpp>

LActorMode::LActorMode()
{
	mRoomChanged = false;
}

void LActorMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	
	ImGui::Begin("sceneHierarchy", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

	ImGui::Text("Rooms");
	ImGui::SameLine();
	
	bool roomsUpdated = false;

	if(ImGui::Button("+")){

		// This should probably pop open a modal but doing so from here is ass.

		// First find a hole in the room indices if it exists
		std::vector<std::shared_ptr<LRoomDOMNode>> rooms = current_map->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);

		int32_t newRoomIdx, newRoomId;
		newRoomIdx = newRoomId = rooms.size();
	
		std::sort(rooms.begin(), rooms.end(), [&](std::shared_ptr<LRoomDOMNode> l, std::shared_ptr<LRoomDOMNode> r){
			return l->GetRoomIndex() < r->GetRoomIndex();
		});

		for(int i = 1; i < rooms.size(); i++){
			if(rooms[i]->GetRoomIndex() - rooms[i-1]->GetRoomIndex() > 1){
				newRoomIdx = rooms[i-1]->GetRoomIndex() + 1;
			}
		}

		// Now find hole in Room IDs if it exists. The fact that these do not line up entirely is a real pain and also I don't know why? 
		std::sort(rooms.begin(), rooms.end(), [&](std::shared_ptr<LRoomDOMNode> l, std::shared_ptr<LRoomDOMNode> r){
			return l->GetRoomID() < r->GetRoomID();
		});

		for(int i = 1; i < rooms.size(); i++){
			if(rooms[i]->GetRoomID() - rooms[i-1]->GetRoomID() > 1){
				newRoomId = rooms[i-1]->GetRoomIndex() + 1;
			}
		}

		std::shared_ptr<LRoomDOMNode> newRoom = std::make_shared<LRoomDOMNode>(fmt::format("room_{:02}", newRoomIdx));
		std::shared_ptr<LRoomDataDOMNode> newRoomData = std::make_shared<LRoomDataDOMNode>(fmt::format("room_{:02}_data", newRoomIdx));
		std::shared_ptr<LTreasureTableDOMNode> roomChest = std::make_shared<LTreasureTableDOMNode>("chest");
		
		newRoom->SetRoomNumber(newRoomId);
		newRoomData->SetRoomID(newRoomId);

		newRoomData->SetRoomIndex(newRoomIdx);
		//Hardcode part of the res path for now. Map should probably have some way to get resource path.
		newRoomData->SetRoomResourcePath(fmt::format("/Iwamoto/{1}/room_{0:02}.arc", newRoomIdx, current_map->GetName()));

		//TODO: add default room archive?

		newRoom->AddChild(roomChest);
		newRoom->AddChild(newRoomData);
		current_map->AddChild(newRoom);

		//Add a flag to regenerate path rendering
		roomsUpdated = true;
	}

	ImGui::SameLine();
	
	if(ImGui::Button("-")){
		if(mSelectionManager.GetPrimarySelection().get()->GetNodeType() == EDOMNodeType::Room){
			current_map->RemoveChild(mSelectionManager.GetPrimarySelection());
			mSelectionManager.ClearSelection();
		}
		roomsUpdated = true;
	}

	if(roomsUpdated){
		std::sort(current_map->Children.begin(), current_map->Children.end(), [&](std::shared_ptr<LDOMNodeBase> l, std::shared_ptr<LDOMNodeBase> r){
			if((l->GetNodeType() == r->GetNodeType()) && l->GetNodeType() == EDOMNodeType::Room){
				return static_pointer_cast<LRoomDOMNode>(l)->GetRoomNumber() < static_pointer_cast<LRoomDOMNode>(r)->GetRoomNumber();
			} else {
				return false;
			}
		});
	}
	
	ImGui::Separator();

	ImGui::Text("Current Room");
	mRoomChanged = LUIUtility::RenderNodeReferenceCombo("##selectedRoom", EDOMNodeType::Room, current_map, mManualRoomSelect);

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
	ImGui::Begin("detailWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

	ImGui::Text("Object Details");
	ImGui::Separator();

	if (mSelectionManager.IsMultiSelection())
		ImGui::Text("[Multiple Selection]");
	else if (mSelectionManager.GetPrimarySelection() != nullptr)
		std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);

	ImGui::End();
}

void LActorMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{
	//LUIUtility::RenderGizmoToggle();
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
	
	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}

	if(ImGui::IsKeyPressed(ImGuiKey_1)){
		mGizmoMode = ImGuizmo::OPERATION::TRANSLATE;
	} else if (ImGui::IsKeyPressed(ImGuiKey_2)){
		mGizmoMode = ImGuizmo::OPERATION::ROTATE;
	} else if (ImGui::IsKeyPressed(ImGuiKey_3)){
		mGizmoMode = ImGuizmo::OPERATION::SCALE;
	}

	if(mSelectionManager.GetPrimarySelection() != nullptr){
		
		if(mPreviousSelection == nullptr || mPreviousSelection != mSelectionManager.GetPrimarySelection()){
			mPreviousSelection = mSelectionManager.GetPrimarySelection();
			if(!mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).expired() && !renderer_scene->HasRoomLoaded(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock()->GetRoomNumber())){
				renderer_scene->SetRoom(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock());				
				mManualRoomSelect = mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room);
			}
		}

		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		if(mSelectionManager.GetPrimarySelection().get()->GetNodeType() != EDOMNodeType::Room){
			glm::mat4* m = static_cast<LBGRenderDOMNode*>(mSelectionManager.GetPrimarySelection().get())->GetMat();
			if(ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::LOCAL, &(*m)[0][0], NULL, NULL)){
				renderer_scene->UpdateRenderers();
			}
		} else {
			std::shared_ptr<LRoomDataDOMNode> data = mSelectionManager.GetPrimarySelection()->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();


			glm::mat4 mat = glm::identity<glm::mat4>();
			
			if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
				mat = glm::translate(mat, data->GetMax());
			} else {
				mat = glm::translate(mat, data->GetMin());
			}

			if(ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::LOCAL, &(mat)[0][0], NULL, NULL)){
				renderer_scene->UpdateRenderers();

				if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
					data->SetMax(mat[3]);
				} else {
					data->SetMin(mat[3]);
				}
			}

		}
	}

	if(mRoomChanged){
		
		if(std::shared_ptr<LRoomDOMNode> room = mManualRoomSelect.lock()){
			/*
			std::string room_name = fmt::format("Editing {}", room->GetName());

			Discord::RichPresence.details = room_name.c_str();
			Discord_UpdatePresence(&Discord::RichPresence);
			*/
			renderer_scene->SetRoom(room);
		}
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
