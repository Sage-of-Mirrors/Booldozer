#include "modes/PathMode.hpp"
#include "DOM/PathDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"

#include "imgui.h"
#include "imgui_internal.h"

LPathMode::LPathMode()
{
	mGizmoMode = ImGuizmo::OPERATION::TRANSLATE;
}

void LPathMode::RenderSceneHierarchy(std::shared_ptr<LMapDOMNode> current_map)
{
	ImGui::Begin("sceneHierarchy");
	ImGui::Text("Rooms");
	ImGui::Separator();

	auto rooms = current_map->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	for (auto r : rooms)
	{
		bool roomTreeOpened = ImGui::TreeNode(r->GetName().c_str());
		if (ImGui::BeginPopupContextItem())
		{
			RenderRoomContextMenu(r);
		}
		if (ImGui::BeginDragDropTarget())
		{
			auto pathNode = GetPathDragDropNode();

			if (pathNode != nullptr)
			{
				auto sharedPathNode = pathNode->GetSharedPtr<LPathDOMNode>(EDOMNodeType::Path);
				r->AddChild(sharedPathNode);
				sharedPathNode->SetRoomNumber(r->GetRoomNumber());
			}
		}

		if (roomTreeOpened)
		{
			ImGui::Indent();

			auto paths = r->GetChildrenOfType<LPathDOMNode>(EDOMNodeType::Path);
			for (size_t i = 0; i < paths.size(); i++)
			{
				auto path = paths[i];
				std::string nodeName = fmt::format("{0}###{1}", path->GetName(), i);

				bool treeSelected = false;
				bool treeOpened = LUIUtility::RenderNodeSelectableTreeNode(nodeName, path->GetIsSelected() && !bLastClickedWasPoint, treeSelected);
				if (ImGui::BeginPopupContextItem())
				{
					// This path was removed, so skip everything else.
					if (RenderPathContextMenu(std::static_pointer_cast<LPathDOMNode>(path)))
					{
						continue;
					}
				}
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					LPathDOMNode* pathPtr = path.get();
					ImGui::SetDragDropPayload("DOM_NODE_PATH", &pathPtr, sizeof(LDOMNodeBase*));
					ImGui::Text("%s", path->GetName().c_str());
					ImGui::EndDragDropSource();
				}

				if (treeSelected)
				{
					mSelectionManager.AddToSelection(path);
					bLastClickedWasPoint = false;
				}
				if (treeOpened)
				{
					ImGui::Indent();

					for (size_t j = 0; j < path->Children.size(); j++)
					{
						auto point = path->Children[j];
						std::string ptName = fmt::format("point###{0}", j);//fmt::format("Point ", j, ": (", point->X.Value, ", ", point->Y.Value, ", ", point->Z.Value, ")###", j);

						if (LUIUtility::RenderNodeSelectable(point.get(), point->GetIsSelected() && bLastClickedWasPoint))
						{
							mPointSelection.AddToSelection(point);
							bLastClickedWasPoint = true;
						}
						if (ImGui::BeginPopupContextItem(fmt::format("pointctx###{0}", j).c_str()))
						{
							// This point was removed, so skip everything else.
							if (RenderPointContextMenu(std::static_pointer_cast<LPathDOMNode>(path), std::static_pointer_cast<LPathPointDOMNode>(point)))
							{
								continue;
							}
						}
					}

					ImGui::Unindent();
					ImGui::TreePop();
				}
			}

			ImGui::Unindent();
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void LPathMode::RenderDetailsWindow()
{
	ImGui::Begin("detailWindow");

	ImGui::Text("Point Settings");
	ImGui::Separator();

	// Show point details
	if (bLastClickedWasPoint)
	{
		if (mPointSelection.IsMultiSelection())
			ImGui::Text("[Multiple Selection]");
		else if (mPointSelection.GetPrimarySelection() != nullptr)
			std::static_pointer_cast<LUIRenderDOMNode>(mPointSelection.GetPrimarySelection())->RenderDetailsUI(0);
	}
	// Show path details
	else
	{
		if (mSelectionManager.IsMultiSelection())
			ImGui::Text("[Multiple Selection]");
		else if (mSelectionManager.GetPrimarySelection() != nullptr)
			std::static_pointer_cast<LUIRenderDOMNode>(mSelectionManager.GetPrimarySelection())->RenderDetailsUI(0);
	}

	ImGui::End();
}

void LPathMode::Render(std::shared_ptr<LMapDOMNode> current_map, LEditorScene* renderer_scene)
{
	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderSceneHierarchy(current_map);

	ImGui::SetNextWindowClass(&mainWindowOverride);
	RenderDetailsWindow();

	for(auto& node : current_map.get()->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
		node->RenderBG(0);
	}
}

void LPathMode::RenderGizmo(LEditorScene* renderer_scene){
	if (mPointSelection.GetPrimarySelection() != nullptr)
	{
		if (mPreviousSelection == nullptr || mPreviousSelection != mPointSelection.GetPrimarySelection()) {
			mPreviousSelection = mPointSelection.GetPrimarySelection();
			if (!mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).expired() && !renderer_scene->HasRoomLoaded(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock()->GetRoomNumber())) {
				renderer_scene->SetRoom(mPreviousSelection->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock());
			}
		}

		glm::mat4* m = ((LBGRenderDOMNode*)(mPointSelection.GetPrimarySelection().get()))->GetMat();
		glm::mat4 view = renderer_scene->getCameraView();
		glm::mat4 proj = renderer_scene->getCameraProj();

		bool moved = ImGuizmo::Manipulate(&view[0][0], &proj[0][0], mGizmoMode, ImGuizmo::WORLD, &(*m)[0][0], NULL, NULL);

		if(mPointSelection.GetPrimarySelection()->GetNodeType() == EDOMNodeType::PathPoint && moved){
			renderer_scene->UpdateRenderers();
		}
	}
}

bool LPathMode::RenderPointContextMenu(std::shared_ptr<LPathDOMNode> path, std::shared_ptr<LPathPointDOMNode> point)
{
	bool deleted = false;

	if (ImGui::Selectable("Insert Point Above"))
	{
		ptrdiff_t index = LGenUtility::VectorIndexOf(path->Children, std::static_pointer_cast<LDOMNodeBase>(point));
		path->AddChildAtIndex(std::make_shared<LPathPointDOMNode>("Path Point"), index);
	}

	if (ImGui::Selectable("Insert Point Below"))
	{
		ptrdiff_t index = LGenUtility::VectorIndexOf(path->Children, std::static_pointer_cast<LDOMNodeBase>(point));
		path->AddChildAtIndex(std::make_shared<LPathPointDOMNode>("Path Point"), index + 1);
	}

	ImGui::Separator();

	if (ImGui::Selectable("Delete"))
	{
		path->RemoveChild(point);

		if (mPointSelection.GetPrimarySelection() == point)
			mPointSelection.ClearSelection();

		deleted = true;
	}

	ImGui::EndPopup();
	return deleted;
}

bool LPathMode::RenderPathContextMenu(std::shared_ptr<LPathDOMNode> path)
{
	bool deleted = false;
	
	if (auto pathParentLocked = path->Parent.lock())
	{
		if (ImGui::Selectable("Add Point"))
		{
			path->AddChild(std::make_shared<LPathPointDOMNode>("Path Point"));
		}

		ImGui::Separator();

		if (ImGui::Selectable("Delete"))
		{
			pathParentLocked->RemoveChild(path);

			if (mSelectionManager.GetPrimarySelection() == path)
				mSelectionManager.ClearSelection();

			deleted = true;
		}
	}

	ImGui::EndPopup();
	return deleted;
}

void LPathMode::RenderRoomContextMenu(std::shared_ptr<LRoomDOMNode> room)
{
	if (ImGui::Selectable("Add Path"))
	{
		auto newPath = std::make_shared<LPathDOMNode>("Path");
		room->AddChild(newPath);
		mSelectionManager.AddToSelection(newPath);
	}

	ImGui::EndPopup();
}

void LPathMode::OnBecomeActive()
{
	std::cout << "[Booldozer]: Path mode switching in!" << std::endl;
}

void LPathMode::OnBecomeInactive()
{
	std::cout << "[Booldozer]: Path mode switching out!" << std::endl;
}

LPathDOMNode* LPathMode::GetPathDragDropNode()
{
	const ImGuiPayload* payload = ImGui::GetDragDropPayload();

	if (payload != nullptr && payload->Data != nullptr)
	{
		if (payload->IsDataType("DOM_NODE_PATH"))
		{
			if (ImGui::AcceptDragDropPayload(payload->DataType) == nullptr)
				return nullptr;

			IM_ASSERT(payload->DataSize == sizeof(LDOMNodeBase*));
			return *static_cast<LPathDOMNode**>(payload->Data);
		}
	}

	return nullptr;
}
