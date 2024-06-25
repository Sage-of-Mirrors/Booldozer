#include "modes/ActorMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "IconsForkAwesome.h"
#include "UIUtil.hpp"
#include "Options.hpp"

//#include <DiscordIntegration.hpp>

bool isRoomDirty = false;
std::shared_ptr<LRoomDOMNode> prevRoom = nullptr;

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

	ImGui::Text(ICON_FK_PLUS_CIRCLE);
	if(ImGui::IsItemClicked(0)){
		// Show new room modal!
		auto rooms = current_map->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
		std::shared_ptr<LRoomDOMNode> newRoom = std::make_shared<LRoomDOMNode>(fmt::format("Room {}", rooms.size() + 1));
		std::shared_ptr<LRoomDataDOMNode> newRoomData = std::make_shared<LRoomDataDOMNode>(fmt::format("Room {}", rooms.size() + 1));

		std::string resourcePathRoot = std::filesystem::path(rooms[0]->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0]->GetResourcePath()).parent_path().string();

		newRoomData->SetRoomResourcePath(fmt::format("{}/room_{:02}.arc", resourcePathRoot, rooms.size() + 1));

		// for some reason filesystem path wasnt working to build this path so for now just build it manually...
		std::string resPathInRoot = fmt::format("{}/{}/{}", OPTIONS.mRootPath, "files", newRoomData->GetResourcePath());
		std::cout << "[ActorMode]: Room resource path " << resPathInRoot << std::endl;
		if(!std::filesystem::exists(resPathInRoot)){
			std::shared_ptr<Archive::Rarc> arc = Archive::Rarc::Create();
			std::shared_ptr<Archive::Folder> root = Archive::Folder::Create(arc);
			arc->SetRoot(root);
			// now before save, construct as path to replace directory seperators with proper system seps
			arc->SaveToFile(std::filesystem::path(resPathInRoot).string());
		}

		newRoomData->SetRoomID(rooms.size()+1);
		newRoomData->SetRoomIndex(rooms.size()+1);
		newRoom->AddChild(newRoomData);

		newRoom->SetRoomNumber(rooms.size()+1);

		current_map->AddChild(newRoom);
	}

	ImGui::SameLine();
	ImGui::Text(ICON_FK_MINUS_CIRCLE);
	if(ImGui::IsItemClicked(0)){
		if(mSelectionManager.GetPrimarySelection()->GetNodeType() == EDOMNodeType::Room){
			auto room = mSelectionManager.GetPrimarySelection();
			mSelectionManager.ClearSelection();
			current_map->RemoveChild(room);
			ImGui::End();
			return;
		}
		roomsUpdated = true;
	}

	if(roomsUpdated){
		std::sort(current_map->Children.begin(), current_map->Children.end(), [&](std::shared_ptr<LDOMNodeBase> l, std::shared_ptr<LDOMNodeBase> r){
			if((l->GetNodeType() == r->GetNodeType()) && l->GetNodeType() == EDOMNodeType::Room){
				return static_pointer_cast<LRoomDOMNode>(l)->GetRoomIndex() < static_pointer_cast<LRoomDOMNode>(r)->GetRoomIndex();
			} else {
				return false;
			}
		});
	}

	LUIUtility::RenderTooltip("EXPERIMENTAL: Please *backup rooms.map*, as room ids/indicies may get shuffled by accident!");
	
	ImGui::Separator();

	ImGui::Text("Current Room");
	mRoomChanged = LUIUtility::RenderNodeReferenceCombo("##selectedRoom", EDOMNodeType::Room, current_map, mManualRoomSelect);

	uint32_t i = 0;
	current_map->ForEachChildOfType<LRoomDOMNode>(EDOMNodeType::Room, [i, this](std::shared_ptr<LRoomDOMNode> room) mutable {
		ImGui::PushID(i);
		room->RenderHierarchyUI(room, &mSelectionManager);
		ImGui::PopID();
		i++;
	});

	std::vector<std::shared_ptr<LMapCollisionDOMNode>> collision = current_map->GetChildrenOfType<LMapCollisionDOMNode>(EDOMNodeType::MapCollision);

	if(!collision.empty()){
		ImGui::PushID(i);
		collision[0]->RenderHierarchyUI(collision[0], &mSelectionManager);
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

	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map);

	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderDetailsWindow();

	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}

	if(mRoomChanged || isRoomDirty){
		if(std::shared_ptr<LRoomDOMNode> room = mManualRoomSelect.lock()){
			/*
			std::string room_name = fmt::format("Editing {}", room->GetName());

			Discord::RichPresence.details = room_name.c_str();
			Discord_UpdatePresence(&Discord::RichPresence);
			*/
			renderer_scene->SetRoom(room);
			isRoomDirty = false;
		}
	}

}

void LActorMode::RenderGizmo(LEditorScene* renderer_scene){
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
		
		if(mSelectionManager.GetPrimarySelection()->GetNodeType() != EDOMNodeType::Room){
			glm::mat4* m = static_cast<LBGRenderDOMNode*>(mSelectionManager.GetPrimarySelection().get())->GetMat();
			glm::mat4 delta(1.0);
			if(ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::WORLD, &(*m)[0][0], &delta[0][0], NULL)){
				for(auto node : mSelectionManager.GetSelection()){
					if(node != mSelectionManager.GetPrimarySelection()){
						(*dynamic_pointer_cast<LBGRenderDOMNode>(node)->GetMat()) = (*dynamic_pointer_cast<LBGRenderDOMNode>(node)->GetMat()) * delta;
					}
					EDOMNodeType type = node->GetNodeType();
					if(type == EDOMNodeType::PathPoint || type == EDOMNodeType::Event  || type == EDOMNodeType::Observer || type == EDOMNodeType::Object){
						renderer_scene->UpdateRenderers();
						break;
					}
				}
			}
		} else {
			std::shared_ptr<LRoomDOMNode> curRoom = dynamic_pointer_cast<LRoomDOMNode>(mSelectionManager.GetPrimarySelection());
			if(prevRoom != curRoom){
				renderer_scene->SetRoom(dynamic_pointer_cast<LRoomDOMNode>(mSelectionManager.GetPrimarySelection()));
				prevRoom = curRoom;
			}

			std::shared_ptr<LRoomDataDOMNode> data = mSelectionManager.GetPrimarySelection()->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

			glm::mat4 mat = glm::identity<glm::mat4>();
			glm::mat4 deltaMat = glm::identity<glm::mat4>();
			
			if(ImGui::IsKeyDown(ImGuiKey_LeftAlt)){
				mat = glm::translate(mat, data->GetMin());
			}
			else if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
				mat = glm::translate(mat, data->GetMax());
			} else {
				glm::vec3 max = data->GetMax();
				glm::vec3 min = data->GetMin();
				mat = glm::translate(mat, {(max.x + min.x) / 2, (max.y + min.y) / 2, (max.z + min.z) / 2});
			}

			if(ImGuizmo::Manipulate(&view[0][0], &proj[0][0], ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::WORLD, &(mat)[0][0], &(deltaMat)[0][0], NULL)){
				renderer_scene->UpdateRenderers();
				
				if(ImGui::IsKeyDown(ImGuiKey_LeftAlt)){
					data->SetMin(mat[3]);
				}
				else if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
					data->SetMax(mat[3]);
				} else {
					glm::vec3 translation = glm::vec3(deltaMat[3]);
					// add delta to max and min
					data->SetMax(data->GetMax() + translation);
					data->SetMin(data->GetMin() + translation);
					std::shared_ptr<LRoomDOMNode> roomNode = dynamic_pointer_cast<LRoomDOMNode>(mSelectionManager.GetPrimarySelection());
					roomNode->SetRoomModelDelta(roomNode->GetRoomModelDelta() + translation);
					// add delta to all child transforms
					for(auto child : mSelectionManager.GetPrimarySelection()->GetChildrenOfType<LEntityDOMNode>(EDOMNodeType::Entity)){
						(*child->GetMat())[3].x += translation.x;
						(*child->GetMat())[3].y += translation.y;
						(*child->GetMat())[3].z += translation.z;

						child->SetPosition(child->GetPosition() + translation);
					}
				}
			}

		}
	}
}

void LActorMode::OnBecomeActive()
{
	std::cout << "[Booldozer]: Actor mode switching in!\n" << std::endl;
}

void LActorMode::OnBecomeInactive()
{
	std::cout << "[Booldozer]: Actor mode switching out!\n" << std::endl;
}
