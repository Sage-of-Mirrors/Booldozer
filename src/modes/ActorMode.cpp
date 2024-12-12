#include "modes/ActorMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "IconsForkAwesome.h"
#include "UIUtil.hpp"
#include "Options.hpp"
#include "history/Mat4HistoryItem.hpp"
#include "history/RoomBoundsHistoryItem.hpp"
#include "history/RoomMoveHistoryItem.hpp"
#include <glm/gtx/matrix_decompose.hpp>

//#include <DiscordIntegration.hpp>

bool isRoomDirty = false;
bool roomsUpdated = false;
std::shared_ptr<LRoomDOMNode> prevRoom = nullptr;

LActorMode::LActorMode()
{
	mRoomChanged = false;
}

void LActorMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map, EEditorMode& mode)
{
	
	ImGui::Begin("sceneHierarchy", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

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

	ImGui::Text("Rooms");
	ImGui::SameLine();

	ImGui::Text(ICON_FK_PLUS_CIRCLE);
	if(ImGui::IsItemClicked(0)){
		// Show new room modal!
		auto rooms = current_map->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
		std::shared_ptr<LRoomDOMNode> newRoom = std::make_shared<LRoomDOMNode>(std::format("Room {}", rooms.size() + 1));
		std::shared_ptr<LRoomDataDOMNode> newRoomData = std::make_shared<LRoomDataDOMNode>(std::format("Room {}", rooms.size() + 1));

		std::string resourcePathRoot = "";
		if(rooms.size() == 0){
			resourcePathRoot = (std::filesystem::path("Iwamoto") / std::format("map{}", current_map->GetMapNumber())).string();
		} else {
			resourcePathRoot = std::filesystem::path(rooms[0]->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0]->GetResourcePath()).parent_path().string();
		}

		newRoomData->SetRoomResourcePath(std::format("{}/room_{:02}.arc", resourcePathRoot, rooms.size() + 1));

		// for some reason filesystem path wasnt working to build this path so for now just build it manually...
		std::string resPathInRoot = std::format("{}/{}/{}", OPTIONS.mRootPath, "files", newRoomData->GetResourcePath());
		LGenUtility::Log << "[ActorMode]: Room resource path " << resPathInRoot << std::endl;
		if(!std::filesystem::exists(resPathInRoot)){
			std::shared_ptr<Archive::Rarc> arc = Archive::Rarc::Create();
			std::shared_ptr<Archive::Folder> root = Archive::Folder::Create(arc);
			arc->SetRoot(root);
			// now before save, construct as path to replace directory seperators with proper system seps
			arc->SaveToFile(std::filesystem::path(resPathInRoot).string());
		}

		newRoomData->SetRoomID(rooms.size());
		newRoomData->SetRoomIndex(rooms.size());
		newRoomData->GetAdjacencyList().push_back(newRoom);
		newRoom->AddChild(newRoomData);

		newRoom->SetRoomNumber(rooms.size());

		current_map->AddChild(newRoom);
	}

	ImGui::SameLine();
	ImGui::Text(ICON_FK_MINUS_CIRCLE);
	if(ImGui::IsItemClicked(0)){
		if(mSelectionManager.GetPrimarySelection()->GetNodeType() == EDOMNodeType::Room){
			auto room = std::dynamic_pointer_cast<LRoomDOMNode>(mSelectionManager.GetPrimarySelection());
			mSelectionManager.ClearSelection();
			current_map->ForEachChildOfType<LRoomDOMNode>(EDOMNodeType::Room, [&](std::shared_ptr<LRoomDOMNode> r){
				r->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0]->RemoveAdjacent(room);
			});
			current_map->RemoveChild(room);
			roomsUpdated = true;
			ImGui::End();
			return;
		}
	}

	LUIUtility::RenderTooltip("EXPERIMENTAL: Please *backup rooms.map*, as room ids/indicies may get shuffled by accident!");
	//ImGui::Separator();

	//ImGui::Text("Current Room");
	//mRoomChanged = LUIUtility::RenderNodeReferenceCombo("##selectedRoom", EDOMNodeType::Room, current_map, mManualRoomSelect);

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

	if(mPreviousSelection == mSelectionManager.GetPrimarySelection()){
		if (mSelectionManager.IsMultiSelection()){
			ImGui::Text("[Multiple Selection]");
			/* Some WIP multi selection
			ImGui::BeginTabBar("##multiSelectionTabs");

			for(auto item : mSelectionManager.GetSelection()){
				if(ImGui::BeginTabItem(std::format("{}##{}", item->GetName(), item->GetID()).c_str())){
					std::static_pointer_cast<LUIRenderDOMNode>(item)->RenderDetailsUI(0);
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
			*/
		}
		else if (mSelectionManager.GetPrimarySelection() != nullptr)
			std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);
	} else if(mPreviousSelection != nullptr){
		std::static_pointer_cast<LUIRenderDOMNode>(mPreviousSelection)->RenderDetailsUI(0);
	}

	ImGui::End();
}

void LActorMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene, EEditorMode& mode)
{

	//LUIUtility::RenderGizmoToggle();

	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map, mode);

	if(roomsUpdated){
		renderer_scene->SetRoom(current_map->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room).front());
		roomsUpdated = false;
	}

	ImGui::SetNextWindowClass(&mainWindowOverride);
	
	RenderDetailsWindow();
	
	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}

	if(mRoomChanged || isRoomDirty){
		if(std::shared_ptr<LRoomDOMNode> room = mManualRoomSelect.lock()){
			renderer_scene->SetRoom(room);
			isRoomDirty = false;
		}
	}

}

void LActorMode::RenderGizmo(LEditorScene* renderer_scene){
	if(mSelectionManager.GetPrimarySelection() != nullptr){
		if(mPreviousSelection != nullptr && mPreviousSelection != mSelectionManager.GetPrimarySelection()){
			ImGui::SetWindowFocus(nullptr);
			if(!mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).expired() && !renderer_scene->HasRoomLoaded(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock()->GetRoomNumber())){
				renderer_scene->SetRoom(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock());				
				mManualRoomSelect = mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room);
			}
		}

		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();
		
		if(mSelectionManager.GetPrimarySelection()->GetNodeType() != EDOMNodeType::Room){
			glm::mat4* m = static_cast<LBGRenderDOMNode*>(mSelectionManager.GetPrimarySelection().get())->GetMat();
			glm::mat4 delta(1.0f);
			if(ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::WORLD, &(*m)[0][0], &delta[0][0], NULL)){
				if(!mGizmoWasUsing){ // if we we arent already using the gizmo, invert the transform this just did and set that as the original transform
					mOriginalTransform = *m * glm::inverse(delta);
				}
				
				for(auto node : mSelectionManager.GetSelection()){
					if(node != mSelectionManager.GetPrimarySelection()){
						(*dynamic_pointer_cast<LBGRenderDOMNode>(node)->GetMat()) *= delta;
					}

					EDOMNodeType type = node->GetNodeType();
					if(type == EDOMNodeType::PathPoint || type == EDOMNodeType::Event  || type == EDOMNodeType::Observer || type == EDOMNodeType::Object){
						renderer_scene->SetDirty();
						break;
					}
				}

				mGizmoWasUsing = true;
			}

			if(!ImGuizmo::IsUsing() && mGizmoWasUsing){ //finished using the gizmo, add a history item

				// snap objects to room bounds
				std::shared_ptr<LRoomDataDOMNode> curRoom = mSelectionManager.GetPrimarySelection()->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock()->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];
				for(auto node : mSelectionManager.GetSelection()){
					glm::mat4* transform = dynamic_pointer_cast<LBGRenderDOMNode>(node)->GetMat();
					if((*transform)[3].x < curRoom->GetMin().x){
						(*transform)[3].x = curRoom->GetMin().x + 0.01f;
					} else if((*transform)[3].x > curRoom->GetMax().x){
						(*transform)[3].x = curRoom->GetMax().x - 0.01f;
					}
					if((*transform)[3].y < curRoom->GetMin().y){
						(*transform)[3].y = curRoom->GetMin().y + 0.01f;
					} else if((*transform)[3].y > curRoom->GetMax().y){
						(*transform)[3].y = curRoom->GetMax().y - 0.01f;
					}
					if((*transform)[3].z < curRoom->GetMin().z){
						(*transform)[3].z = curRoom->GetMin().z + 0.01f;
					} else if((*transform)[3].z > curRoom->GetMax().z){
						(*transform)[3].z = curRoom->GetMax().z - 0.01f;
					}
				}

				mHistoryManager.AddUndoItem(std::make_shared<LMat4HistoryItem>(std::static_pointer_cast<LBGRenderDOMNode>(mSelectionManager.GetPrimarySelection()), mOriginalTransform));
				mGizmoWasUsing = false;
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
				renderer_scene->SetDirty();
				
				if(ImGui::IsKeyDown(ImGuiKey_LeftAlt)){
					if(!mGizmoWasUsing){
						mGizmoTranslationDelta = data->GetMin();
						mOriginalRoomBoundMin = true;
						mRoomBoundEdited = true;
					}
					data->SetMin(mat[3]);
				}
				else if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
					if(!mGizmoWasUsing){
						mGizmoTranslationDelta = data->GetMax();
						mOriginalRoomBoundMin = false;
						mRoomBoundEdited = true;
					}
					data->SetMax(mat[3]);
				} else {
					mRoomBoundEdited = false;
					glm::vec3 translation = glm::vec3(deltaMat[3]);
					mGizmoTranslationDelta += translation;
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
				mGizmoWasUsing = true;
			}

			if(!ImGuizmo::IsUsing() && mGizmoWasUsing){ 
				if(mRoomBoundEdited){
					mHistoryManager.AddUndoItem(std::make_shared<LRoomBoundsHistoryItem>(data, mGizmoTranslationDelta, mOriginalRoomBoundMin, renderer_scene));
				} else {
					mHistoryManager.AddUndoItem(std::make_shared<LRoomMoveHistoryItem>(std::static_pointer_cast<LRoomDOMNode>(mSelectionManager.GetPrimarySelection()), mGizmoTranslationDelta, renderer_scene));
				}
				mGizmoTranslationDelta = {0,0,0};
				mGizmoWasUsing = false;
			}

		}
	}

	if(!ImGui::GetIO().WantTextInput){
		if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z)){
			mHistoryManager.PerformUndo();
		}

		if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyPressed(ImGuiKey_Z)){
			mHistoryManager.PerformRedo();
		}
	}
	mPreviousSelection = mSelectionManager.GetPrimarySelection();
}

void LActorMode::OnBecomeActive()
{
	LGenUtility::Log << "[Booldozer]: Actor mode switching in!\n" << std::endl;
}

void LActorMode::OnBecomeInactive()
{
	LGenUtility::Log << "[Booldozer]: Actor mode switching out!\n" << std::endl;
}
