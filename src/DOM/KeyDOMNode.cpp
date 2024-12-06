#include "DOM/KeyDOMNode.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "UIUtil.hpp"

LKeyDOMNode::LKeyDOMNode(std::string name) : LKeyDOMNode(name, false)
{

}

LKeyDOMNode::LKeyDOMNode(std::string name, bool isBlackoutKey) : Super(name, isBlackoutKey),
    mCreateName("----"), mCodeName("(null)"), mOpenDoorNumber(0), mSpawnFlag(0), mDespawnFlag(0),
    mCondType(EConditionType::Always_True), mAppearType(EKeyAppearType::Normal), mIsVisible(true)
{
    mType = EDOMNodeType::Key;
    mRoomNumber = -1;
}

void LKeyDOMNode::RenderDetailsUI(float dt)
{
    LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

    // Strings
    LUIUtility::RenderTextInput("Name", &mName);
    LUIUtility::RenderTooltip("What kind of key to spawn.");

    LUIUtility::RenderTextInput("Spawn Group", &mCreateName);
    LUIUtility::RenderTooltip("What Spawn Group this key is in. Set this to ---- to spawn when Luigi enters the room. This is also known as create_name.");

    LUIUtility::RenderTextInput("Script Name", &mCodeName);
    LUIUtility::RenderTooltip("The name that can be used to reference this key in an event.");

    // Integers
    ImGui::InputInt("Door ID", &mOpenDoorNumber);
    LUIUtility::RenderTooltip("The door that this key unlocks.");

    ImGui::InputInt("Spawn Flag", &mSpawnFlag);
    LUIUtility::RenderTooltip("The flag that must be set before this key will begin spawning.");
    ImGui::InputInt("Despawn Flag", &mDespawnFlag);
    LUIUtility::RenderTooltip("If this flag is set, this key will no longer spawn.");

    // Comboboxes
    LUIUtility::RenderComboEnum<EConditionType>("Spawn Condition", mCondType);
    LUIUtility::RenderTooltip("The condition governing whether the key is allowed to spawn.");

    LUIUtility::RenderComboEnum<EKeyAppearType>("Spawn Type", mAppearType);
    LUIUtility::RenderTooltip("The way in which the key will spawn. Using the second option WILL crash the game.");

    // Bools
    LUIUtility::RenderCheckBox("Visible", &mIsVisible);
    LUIUtility::RenderTooltip("Whether this key should be visible.");
}

void LKeyDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    JmpIO->SetString(entry_index, "name", mName);
    JmpIO->SetString(entry_index, "create_name", mCreateName);
    JmpIO->SetString(entry_index, "CodeName", mCodeName);

    JmpIO->SetFloat(entry_index, "pos_x", mPosition.z);
    JmpIO->SetFloat(entry_index, "pos_y", mPosition.y);
    JmpIO->SetFloat(entry_index, "pos_z", mPosition.x);

    JmpIO->SetFloat(entry_index, "dir_x", mRotation.z);
    JmpIO->SetFloat(entry_index, "dir_y", -mRotation.y);
    JmpIO->SetFloat(entry_index, "dir_z", mRotation.x);

    JmpIO->SetFloat(entry_index, "scale_x", mScale.z);
    JmpIO->SetFloat(entry_index, "scale_y", mScale.y);
    JmpIO->SetFloat(entry_index, "scale_z", mScale.x);

    JmpIO->SetUnsignedInt(entry_index, "OpenDoorNo", mOpenDoorNumber);

    JmpIO->SetUnsignedInt(entry_index, "appear_flag", mSpawnFlag);
    JmpIO->SetUnsignedInt(entry_index, "disappear_flag", mDespawnFlag);

    JmpIO->SetUnsignedInt(entry_index, "cond_type", (uint32_t)mCondType);
    JmpIO->SetUnsignedInt(entry_index, "appear_type", (uint32_t)mAppearType);

    JmpIO->SetBoolean(entry_index, "invisible", mIsVisible);
}

void LKeyDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    mName = JmpIO->GetString(entry_index, "name");
    mCreateName = JmpIO->GetString(entry_index, "create_name");
    mCodeName = JmpIO->GetString(entry_index, "CodeName");

    mPosition.z = JmpIO->GetFloat(entry_index, "pos_x");
    mPosition.y = JmpIO->GetFloat(entry_index, "pos_y");
    mPosition.x = JmpIO->GetFloat(entry_index, "pos_z");

    mRotation.z = JmpIO->GetFloat(entry_index, "dir_x");
    mRotation.y = -JmpIO->GetFloat(entry_index, "dir_y");
    mRotation.x = JmpIO->GetFloat(entry_index, "dir_z");

    mScale.z = JmpIO->GetFloat(entry_index, "scale_x");
    mScale.y = JmpIO->GetFloat(entry_index, "scale_y");
    mScale.x = JmpIO->GetFloat(entry_index, "scale_z");

    mOpenDoorNumber = JmpIO->GetSignedInt(entry_index, "OpenDoorNo");

    mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
    mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");

    mCondType = (EConditionType)JmpIO->GetUnsignedInt(entry_index, "cond_type");
    mAppearType = (EKeyAppearType)JmpIO->GetUnsignedInt(entry_index, "appear_type");

    mIsVisible = JmpIO->GetBoolean(entry_index, "invisible");
}

void LKeyDOMNode::PostProcess()
{
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		auto rooms = mapNodeLocked->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
		std::shared_ptr<LRoomDOMNode> containingRoom = nullptr;

		for (auto r : rooms)
		{
			auto roomData = r->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

			if (roomData->CheckPointInBounds(mPosition))
			{
				containingRoom = r;
				break;
			}
		}

		if (containingRoom != nullptr)
		{
			LGenUtility::Log << std::format("[KeyDOMNode]: Key going to {0}", containingRoom->GetName()) << std::endl;
			containingRoom->AddChild(GetSharedPtr<LKeyDOMNode>(EDOMNodeType::Key));
			mapNodeLocked->RemoveChild(GetSharedPtr<LKeyDOMNode>(EDOMNodeType::Key));
		}
	}

}

void LKeyDOMNode::PreProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();
}