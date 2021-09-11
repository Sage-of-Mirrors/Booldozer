#include "DOM/ObjectDOMNode.hpp"
#include "UIUtil.hpp"

LObjectDOMNode::LObjectDOMNode(std::string name) : Super(name),
    mPathName("(null)"), mCodeName("(null)"), mSpawnFlag(0), mDespawnFlag(0), mArg0(0), mArg1(0), mArg2(0)
{
    mType = EDOMNodeType::Object;
    mRoomNumber = -1;
}

void LObjectDOMNode::RenderDetailsUI(float dt)
{
    LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

    // Strings
    LUIUtility::RenderTextInput("Name", &mName);
    LUIUtility::RenderTooltip("What kind of actor this generator spawns.");

    LUIUtility::RenderTextInput("Path Name", &mPathName);
    LUIUtility::RenderTooltip("The name of a path file that this enemy will use. Set this to (null) for no path.");

    LUIUtility::RenderTextInput("Script Name", &mCodeName);
    LUIUtility::RenderTooltip("The name that can be used to reference this enemy in an event.");

    // Integers
    ImGui::InputInt("Arg 0", &mArg0);
    LUIUtility::RenderTooltip("Arg 0");
    ImGui::InputInt("Arg 1", &mArg1);
    LUIUtility::RenderTooltip("Arg 1");
    ImGui::InputInt("Arg 2", &mArg2);
    LUIUtility::RenderTooltip("Arg 2");

    ImGui::InputInt("Spawn Flag", &mSpawnFlag);
    LUIUtility::RenderTooltip("The flag that must be set before this enemy will begin spawning.");
    ImGui::InputInt("Despawn Flag", &mDespawnFlag);
    LUIUtility::RenderTooltip("If this flag is set, this enemy will no longer spawn.");
}

void LObjectDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    JmpIO->SetString(entry_index, "name", mName);
    JmpIO->SetString(entry_index, "path_name", mPathName);
    JmpIO->SetString(entry_index, "CodeName", mCodeName);

    JmpIO->SetFloat(entry_index, "pos_x", mPosition.z);
    JmpIO->SetFloat(entry_index, "pos_y", mPosition.y);
    JmpIO->SetFloat(entry_index, "pos_z", mPosition.x);

    JmpIO->SetFloat(entry_index, "dir_x", mRotation.z);
    JmpIO->SetFloat(entry_index, "dir_y", mRotation.y);
    JmpIO->SetFloat(entry_index, "dir_z", mRotation.x);

    JmpIO->SetFloat(entry_index, "scale_x", mScale.z);
    JmpIO->SetFloat(entry_index, "scale_y", mScale.y);
    JmpIO->SetFloat(entry_index, "scale_z", mScale.x);

    JmpIO->SetSignedInt(entry_index, "room_no", mRoomNumber);

    JmpIO->SetSignedInt(entry_index, "arg0", mArg0);
    JmpIO->SetSignedInt(entry_index, "arg1", mArg1);
    JmpIO->SetSignedInt(entry_index, "arg2", mArg2);

    JmpIO->SetSignedInt(entry_index, "appear_flag", mSpawnFlag);
    JmpIO->SetSignedInt(entry_index, "disappear_flag", mDespawnFlag);
}

void LObjectDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    mName = JmpIO->GetString(entry_index, "name");
    mPathName = JmpIO->GetString(entry_index, "path_name");
    mCodeName = JmpIO->GetString(entry_index, "CodeName");

    mPosition.z = JmpIO->GetFloat(entry_index, "pos_x");
    mPosition.y = JmpIO->GetFloat(entry_index, "pos_y");
    mPosition.x = JmpIO->GetFloat(entry_index, "pos_z");

    mRotation.z = JmpIO->GetFloat(entry_index, "dir_x");
    mRotation.y = JmpIO->GetFloat(entry_index, "dir_y");
    mRotation.x = JmpIO->GetFloat(entry_index, "dir_z");

    mScale.z = JmpIO->GetFloat(entry_index, "scale_x");
    mScale.y = JmpIO->GetFloat(entry_index, "scale_y");
    mScale.x = JmpIO->GetFloat(entry_index, "scale_z");

    mRoomNumber = JmpIO->GetSignedInt(entry_index, "room_no");

    mArg0 = JmpIO->GetSignedInt(entry_index, "arg0");
    mArg1 = JmpIO->GetSignedInt(entry_index, "arg1");
    mArg2 = JmpIO->GetSignedInt(entry_index, "arg2");

    mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
    mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");
}

void LObjectDOMNode::PostProcess()
{
    /*
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();
    */
}

void LObjectDOMNode::PreProcess()
{
    /*
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();
    */
}