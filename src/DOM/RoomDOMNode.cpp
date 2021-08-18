#include "DOM/RoomDOMNode.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "DOM/EnemyDOMNode.hpp"
#include "UIUtil.hpp"

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
	if (ImGui::TreeNode(mName.c_str()))
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
				if (ImGui::TreeNode(Groups[i].Name.c_str()))
				{
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DOM_NODE_BASE"))
						{
							IM_ASSERT(payload->DataSize == sizeof(LDOMNodeBase*));
							LDOMNodeBase* payload_n = *(LDOMNodeBase**)payload->Data;
							std::cout << "Dropped " << payload_n->GetName() << std::endl;
						}
					}

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

void LRoomDOMNode::LoadJmpInfo(uint32_t index, LJmpIO* jmp_io)
{
	mInternalName = jmp_io->GetString(index, "name");

	mRoomNumber = jmp_io->GetSignedInt(index, "RoomNo");
	mThunder = jmp_io->GetSignedInt(index, "Thunder");

	mShouldRenderSkybox = jmp_io->GetBoolean(index, "VRbox");

	mDustLevel = jmp_io->GetSignedInt(index, "DustLv");

	mLightColor.r = jmp_io->GetSignedInt(index, "LightColorR");
	mLightColor.g = jmp_io->GetSignedInt(index, "LightColorG");
	mLightColor.b = jmp_io->GetSignedInt(index, "LightColorB");

	mDistance = jmp_io->GetSignedInt(index, "Distance");
	mLv = jmp_io->GetSignedInt(index, "Lv");
	mSoundEchoParameter = jmp_io->GetSignedInt(index, "sound_echo_parameter");
	mSoundRoomCode = jmp_io->GetSignedInt(index, "sound_room_code");

	mSoundRoomSize = jmp_io->GetUnsignedInt(index, "sound_room_size");
}

void LRoomDOMNode::SaveJmpInfo(uint32_t index, LJmpIO* jmp_io)
{
	jmp_io->SetString(index, "name", mInternalName);

	jmp_io->SetSignedInt(index, "RoomNo", mRoomNumber);
	jmp_io->SetSignedInt(index, "Thunder", mThunder);

	jmp_io->SetBoolean(index, "VRbox", mShouldRenderSkybox);

	jmp_io->SetSignedInt(index, "DustLv", mDustLevel);

	jmp_io->SetSignedInt(index, "LightColorR", mLightColor.r);
	jmp_io->SetSignedInt(index, "LightColorG", mLightColor.g);
	jmp_io->SetSignedInt(index, "LightColorB", mLightColor.b);

	jmp_io->SetSignedInt(index, "Distance", mDistance);
	jmp_io->SetSignedInt(index, "Lv", mLv);
	jmp_io->SetSignedInt(index, "sound_echo_parameter", mSoundEchoParameter);
	jmp_io->SetSignedInt(index, "sound_room_code", mSoundRoomCode);

	jmp_io->SetUnsignedInt(index, "sound_room_size", mSoundRoomSize);
}

bool LRoomDOMNode::CompleteLoad(GCarchive* room_arc)
{
	for (uint32_t i = 0; i < LRoomEntityType_Max; i++)
	{
		EDOMNodeType findType = EDOMNodeType::Base;

		switch (i)
		{
			case LRoomEntityType_Characters:
				findType = EDOMNodeType::Character;
				break;
			case LRoomEntityType_Enemies:
				findType = EDOMNodeType::Enemy;
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
			case LRoomEntityType_Observers:
				findType = EDOMNodeType::Observer;
				break;
			case LRoomEntityType_Paths:
				findType = EDOMNodeType::Path;
				break;
			case LRoomEntityType_BlackoutCharacters:
				findType = EDOMNodeType::BlackoutCharacter;
				break;
			case LRoomEntityType_BlackoutEnemies:
				findType = EDOMNodeType::BlackoutEnemy;
				break;
			case LRoomEntityType_BlackoutObservers:
				findType = EDOMNodeType::BlackoutObserver;
				break;
			default:
				break;
		}

		mRoomEntities[i] = GetChildrenOfType<LEntityDOMNode>(findType);
	}

	LObserverGroup defaultGroup;
	defaultGroup.Name = "Default";
	GetEntitiesWithCreateName("----", LRoomEntityType_Enemies, defaultGroup.EntityNodes);
	GetEntitiesWithCreateName("----", LRoomEntityType_Characters, defaultGroup.EntityNodes);
	Groups.push_back(defaultGroup);



	std::vector<std::shared_ptr<LEntityDOMNode>> observers = mRoomEntities[LRoomEntityType_Observers];
	auto it = std::find_if(observers.begin(), observers.end(),
		[=](std::shared_ptr<LEntityDOMNode> const& object) {
			std::shared_ptr<LObserverDOMNode> castObserver = object->GetSharedPtr<LObserverDOMNode>(EDOMNodeType::Observer);
			return (castObserver->GetDoType() == LDoType::Spawn_Create_Name_Entities && castObserver->GetConditionType() != LConditionType::Enemy_Group_Dead && castObserver->GetConditionType() != LConditionType::All_Candles_Lit);
		}
	);

	for (auto obs : observers)
	{
		std::shared_ptr<LObserverDOMNode> castObs = obs->GetSharedPtr<LObserverDOMNode>(EDOMNodeType::Observer);
		if (castObs->GetStringArg() == "(null)")
			continue;

		LObserverGroup newGroup;
		newGroup.Name = "Group \"" + castObs->GetStringArg() + "\"";
		GetEntitiesWithCreateName(castObs->GetStringArg(), LRoomEntityType_Enemies, newGroup.EntityNodes);
		GetEntitiesWithCreateName(castObs->GetStringArg(), LRoomEntityType_Characters, newGroup.EntityNodes);
		Groups.push_back(newGroup);
	}

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
