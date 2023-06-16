#include "DOM/RoomDOMNode.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "DOM/EnemyDOMNode.hpp"
#include "UIUtil.hpp"
#include "Options.hpp"

std::string const LRoomEntityTreeNodeNames[LRoomEntityType_Max] = {
	"Characters",
	"Enemies",
	"Furniture",
	"Generators",
	"Objects",
	"Observers",
	"Paths",
	"Characters (Blackout)",
	"Enemies (Blackout)",
	"Observers (Blackout)"
};

LRoomDOMNode::LRoomDOMNode(std::string name) : LBGRenderDOMNode(name)
{
	mType = EDOMNodeType::Room;
}

void LRoomDOMNode::RenderHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection)
{
	// This checkbox toggles rendering of the room and all of its children.
	LUIUtility::RenderCheckBox(this);
	ImGui::SameLine();

	// Room tree start
	bool treeSelected = false;
	bool treeOpened = LUIUtility::RenderNodeSelectableTreeNode(GetName(), GetIsSelected(), treeSelected);
	if (treeOpened)
	{
		// Iterating all of the entity types
		for (uint32_t i = 0; i < LRoomEntityType_Max; i++)
		{
			// ImGui ID stack is now at <room name>##<i>
			ImGui::PushID(i);

			// This checkbox toggles rendering of this entire category of entities.
			// Since it isn't a real node, we'll manually toggle the entities' visibility.
			if (LUIUtility::RenderCheckBox("##type_is_rendered", &mRoomEntityVisibility[i]))
			{
				for (auto n : mRoomEntities[i])
					n->SetIsRendered(mRoomEntityVisibility[i]);
			}

			// S t y l i n g
			ImGui::SameLine();
			ImGui::Indent();

			// Entity tree <i> start
			if (ImGui::TreeNode(LRoomEntityTreeNodeNames[i].c_str()))
			{
				// Iterating all of the entities of type <i>
				for (uint32_t j = 0; j < mRoomEntities[i].size(); j++)
				{
					// ImGui ID stack is now at <room name>##<i>##<j>
					ImGui::PushID(j);

					// This checkbox toggles rendering of the individual node that
					// we're currently building the UI for.
					LUIUtility::RenderCheckBox(mRoomEntities[i][j].get());
					ImGui::SameLine();

					// Render the current node however it wants to be rendered
					ImGui::Indent();
					mRoomEntities[i][j]->RenderHierarchyUI(mRoomEntities[i][j], mode_selection);
					ImGui::Unindent();

					// ImGui ID stack returns to <room name>##<i>
					ImGui::PopID();
				}

				// End entity tree <i>
				ImGui::TreePop();
			}

			// U n s t y l i n g
			ImGui::Unindent();

			// ImGui ID stack returns to <room name>
			ImGui::PopID();
		}

		// End room tree
		ImGui::TreePop();
	}

	if (treeSelected)
	{
		mode_selection->AddToSelection(GetSharedPtr<LUIRenderDOMNode>(EDOMNodeType::UIRender));
	}
}

void LRoomDOMNode::RenderWaveHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection)
{
	// This checkbox toggles rendering of the room and all of its children.
	LUIUtility::RenderCheckBox(this);
	ImGui::SameLine();

	// Room tree start
	if (ImGui::TreeNode(mName.c_str()))
	{
		if (Groups.size() > 0)
		{
			for (uint32_t i = 0; i < Groups.size(); i++)
			{
				ImGui::PushID(i);

				ImGui::Indent();

				bool bTreeExpanded = ImGui::TreeNode(Groups[i].GetGroupName().c_str());

				// Handle drag and drop
				if (ImGui::BeginDragDropTarget())
				{
					LDOMNodeBase* dragDropNode = GetSpawnGroupDragDropNode();

					// Skip if there's no pending drag and drop to handle
					if (dragDropNode != nullptr)
					{
						std::shared_ptr<LEntityDOMNode> sharedNode = GetSpawnGroupDragDropNode()->GetSharedPtr<LEntityDOMNode>(EDOMNodeType::Entity);
						LSpawnGroup* originGroup = GetSpawnGroupWithCreateName(sharedNode->GetCreateName());

						// Skip if source and destination groups are the same
						if (Groups[i].CreateName != originGroup->CreateName)
						{
							// Remove the node from its original group
							for (auto iter = originGroup->EntityNodes.begin(); iter != originGroup->EntityNodes.end(); ++iter)
							{
								if (*iter == sharedNode)
								{
									originGroup->EntityNodes.erase(iter);
									break;
								}
							}

							// Add the node to its new group and update its create name
							Groups[i].EntityNodes.push_back(sharedNode);
							sharedNode->SetCreateName(Groups[i].CreateName);

							std::cout << "Moved " << dragDropNode->GetName() << " from group " << originGroup->CreateName << " to group " << sharedNode->GetCreateName() << std::endl;
						}
					}

					ImGui::EndDragDropTarget();
				}

				if (bTreeExpanded)
				{
					for (auto entity : Groups[i].EntityNodes)
					{
						ImGui::Indent();
						entity->RenderHierarchyUI(entity, mode_selection);
						ImGui::Unindent();
					}

					ImGui::TreePop();
				}

				ImGui::Unindent();

				ImGui::PopID();
			}
		}

		ImGui::TreePop();
	}
}

void LRoomDOMNode::RenderDetailsUI(float dt)
{
	auto dataNode = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

	// Integers
	ImGui::InputInt("Lightning Frequency", &mThunder);
	LUIUtility::RenderTooltip("How frequently thunder and lightning occurs while in this room.");

	ImGui::InputInt("Dust Intensity", &mDustLevel);
	LUIUtility::RenderTooltip("How dusty this room is.");

	ImGui::InputInt("Light Falloff", &mDistance);
	LUIUtility::RenderTooltip("How far light should reach inside this room.");
	ImGui::InputInt("GBH Color", &mLv);
	LUIUtility::RenderTooltip("The index of the color that this room should be on the GameBoy Horror map.");

	ImGui::InputInt("Echo Intensity", &mSoundEchoParameter);
	LUIUtility::RenderTooltip("How much sound echoes within this room.");
	ImGui::InputInt("Sound Room Code", &mSoundRoomCode);
	LUIUtility::RenderTooltip("???");
	ImGui::InputInt("Sound Room Size", &mSoundRoomSize);
	LUIUtility::RenderTooltip("???");

	// Colors
	ImGui::ColorEdit3("Darkened Color", dataNode->GetDarkColor());
	ImGui::ColorEdit3("Lit Color", mLightColor);

	// Bools
	LUIUtility::RenderCheckBox("Skybox Visible?", &mShouldRenderSkybox);
	LUIUtility::RenderTooltip("Whether the map's skybox should be visible outside this room.");

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::InputInt("Camera Position Index", &mSoundEchoParameter);
	LUIUtility::RenderTooltip("How much sound echoes within this room.");

	LUIUtility::RenderNodeReferenceVector("Adjacent Rooms", EDOMNodeType::Room, Parent, dataNode->GetAdjacencyList());

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	auto chestData = GetChildrenOfType<LTreasureTableDOMNode>(EDOMNodeType::TreasureTable);
	if (chestData.size() == 0)
		return;

	if (ImGui::TreeNode("Treasure Chest Settings"))
	{
		chestData[0]->RenderDetailsUI(dt);
		ImGui::TreePop();
	}
}

void LRoomDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mInternalName = JmpIO->GetString(entry_index, "name");

	mRoomNumber = JmpIO->GetSignedInt(entry_index, "RoomNo");
	mThunder = JmpIO->GetSignedInt(entry_index, "Thunder");

	mShouldRenderSkybox = JmpIO->GetBoolean(entry_index, "VRbox");

	mDustLevel = JmpIO->GetSignedInt(entry_index, "DustLv");

	mLightColor[0] = JmpIO->GetSignedInt(entry_index, "LightColorR") / 255.f;
	mLightColor[1] = JmpIO->GetSignedInt(entry_index, "LightColorG") / 255.f;
	mLightColor[2] = JmpIO->GetSignedInt(entry_index, "LightColorB") / 255.f;

	mDistance = JmpIO->GetSignedInt(entry_index, "Distance");
	mLv = JmpIO->GetSignedInt(entry_index, "Lv");
	mSoundEchoParameter = JmpIO->GetSignedInt(entry_index, "sound_echo_parameter");
	mSoundRoomCode = JmpIO->GetSignedInt(entry_index, "sound_room_code");

	mSoundRoomSize = JmpIO->GetUnsignedInt(entry_index, "sound_room_size");
}

void LRoomDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mInternalName);

	JmpIO->SetSignedInt(entry_index, "RoomNo", mRoomNumber);
	JmpIO->SetSignedInt(entry_index, "Thunder", mThunder);

	JmpIO->SetBoolean(entry_index, "VRbox", mShouldRenderSkybox);

	JmpIO->SetSignedInt(entry_index, "DustLv", mDustLevel);

	JmpIO->SetSignedInt(entry_index, "LightColorR", mLightColor[0] * 255.f);
	JmpIO->SetSignedInt(entry_index, "LightColorG", mLightColor[1] * 255.f);
	JmpIO->SetSignedInt(entry_index, "LightColorB", mLightColor[2] * 255.f);

	JmpIO->SetSignedInt(entry_index, "Distance", mDistance);
	JmpIO->SetSignedInt(entry_index, "Lv", mLv);
	JmpIO->SetSignedInt(entry_index, "sound_echo_parameter", mSoundEchoParameter);
	JmpIO->SetSignedInt(entry_index, "sound_room_code", mSoundRoomCode);

	JmpIO->SetUnsignedInt(entry_index, "sound_room_size", mSoundRoomSize);
}

bool LRoomDOMNode::CompleteLoad()
{
	std::filesystem::path basePath = std::filesystem::path(OPTIONS.mRootPath) / "files";
	auto roomData = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

	std::filesystem::path t = roomData->GetResourcePath();
	std::filesystem::path fullResPath = basePath / t.relative_path();

	if (std::filesystem::exists(fullResPath))
		//std::cout << fmt::format(mName, "{0} has resource at {1}", fullResPath) << std::endl;

	// Load models here

	auto isBlackoutFilter = [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == true; };
	auto isNotBlackoutFilter = [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == false; };

	for (uint32_t i = 0; i < LRoomEntityType_Max; i++)
	{
		EDOMNodeType findType = EDOMNodeType::Base;

		switch (i)
		{
			case LRoomEntityType_Characters:
			case LRoomEntityType_BlackoutCharacters:
				findType = EDOMNodeType::Character;
				break;
			case LRoomEntityType_Enemies:
			case LRoomEntityType_BlackoutEnemies:
				findType = EDOMNodeType::Enemy;
				break;
			case LRoomEntityType_Observers:
			case LRoomEntityType_BlackoutObservers:
				findType = EDOMNodeType::Observer;
				break;
			case LRoomEntityType_Furniture:
				findType = EDOMNodeType::Furniture;
				break;
			case LRoomEntityType_Generators:
				findType = EDOMNodeType::Generator;
				break;
			case LRoomEntityType_Objects:
				findType = EDOMNodeType::Object;
				break;
			case LRoomEntityType_Paths:
				findType = EDOMNodeType::Path;
				break;
			default:
				break;
		}

		if (i == LRoomEntityType_Characters || i == LRoomEntityType_Enemies || i == LRoomEntityType_Observers)
			mRoomEntities[i] = GetChildrenOfType<LEntityDOMNode>(findType, [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == false; });
		else if (i == LRoomEntityType_BlackoutCharacters || i == LRoomEntityType_BlackoutEnemies || i == LRoomEntityType_BlackoutObservers)
			mRoomEntities[i] = GetChildrenOfType<LEntityDOMNode>(findType, [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == true; });
		else
			mRoomEntities[i] = GetChildrenOfType<LEntityDOMNode>(findType);
	}

	LSpawnGroup defaultGroup;
	GetEntitiesWithCreateName("----", LRoomEntityType_Enemies, defaultGroup.EntityNodes);
	GetEntitiesWithCreateName("----", LRoomEntityType_Characters, defaultGroup.EntityNodes);
	Groups.push_back(defaultGroup);

	std::vector<std::shared_ptr<LEntityDOMNode>> observers = mRoomEntities[LRoomEntityType_Observers];
	auto it = std::find_if(observers.begin(), observers.end(),
		[=](std::shared_ptr<LEntityDOMNode> const& object) {
			std::shared_ptr<LObserverDOMNode> castObserver = object->GetSharedPtr<LObserverDOMNode>(EDOMNodeType::Observer);
			return (castObserver->GetDoType() == EDoType::Trigger_Spawn_Group && castObserver->GetConditionType() != EConditionType::Spawn_Group_is_Dead && castObserver->GetConditionType() != EConditionType::All_Candles_are_Lit);
		}
	);

	for (auto obs : observers)
	{
		std::shared_ptr<LObserverDOMNode> castObs = obs->GetSharedPtr<LObserverDOMNode>(EDOMNodeType::Observer);
		if (castObs->GetStringArg() == "(null)")
			continue;

		LSpawnGroup newGroup(castObs->GetStringArg(), castObs);
		GetEntitiesWithCreateName(castObs->GetStringArg(), LRoomEntityType_Enemies, newGroup.EntityNodes);
		GetEntitiesWithCreateName(castObs->GetStringArg(), LRoomEntityType_Characters, newGroup.EntityNodes);
		Groups.push_back(newGroup);
	}

	auto chestData = GetChildrenOfType<LTreasureTableDOMNode>(EDOMNodeType::TreasureTable);
	if (chestData.size() == 0)
		AddChild(std::make_shared<LTreasureTableDOMNode>(""));

	return true;
}

void LRoomDOMNode::GetEntitiesWithCreateName(const std::string CreateName, const LRoomEntityType Type, std::vector<std::shared_ptr<LEntityDOMNode>>& TargetVec)
{
	for (auto object : mRoomEntities[Type])
	{
		if (object->GetCreateName() == CreateName)
			TargetVec.push_back(object);
	}
}

LSpawnGroup* LRoomDOMNode::GetSpawnGroupWithCreateName(std::string createName)
{
	auto it = std::find_if(Groups.begin(), Groups.end(),
		[=](LSpawnGroup const& object) {
			return object.CreateName == createName;
		}
	);

	if (it == Groups.end())
		return nullptr;

	return &(*it);
}

LEntityDOMNode* LRoomDOMNode::GetSpawnGroupDragDropNode()
{
	const ImGuiPayload* payload = ImGui::GetDragDropPayload();

	if (payload != nullptr && payload->Data != nullptr)
	{
		if (payload->IsDataType("DOM_NODE_CHARACTER") || payload->IsDataType("DOM_NODE_ENEMY") || payload->IsDataType("DOM_NODE_KEY") ||
			payload->IsDataType("DOM_NODE_CHARACTER_BLACKOUT") || payload->IsDataType("DOM_NODE_ENEMY_BLACKOUT") || payload->IsDataType("DOM_NODE_KEY_BLACKOUT"))
		{
			if (ImGui::AcceptDragDropPayload(payload->DataType) == nullptr)
				return nullptr;

			IM_ASSERT(payload->DataSize == sizeof(LDOMNodeBase*));
			return *(LEntityDOMNode**)payload->Data;
		}
	}

	return nullptr;
}
