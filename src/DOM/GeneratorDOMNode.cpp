#include "DOM/GeneratorDOMNode.hpp"
#include "UIUtil.hpp"

LGeneratorDOMNode::LGeneratorDOMNode(std::string name) : Super(name),
    mGenType("elfire"), mPathName("(null)"), mCodeName("(null)"), mSpawnFlag(0), mDespawnFlag(0), mActorSpawnRate(0),
    mActorsPerBurst(0), mActorSpawnLimit(0), mArg0(0), mArg1(0), mArg2(0), mArg3(0), mArg4(0), mArg5(0), mArg6(0), 
    mArg7(0), mArg8(0), mStay(false)
{
    mType = EDOMNodeType::Generator;
}

void LGeneratorDOMNode::CopyTo(LGeneratorDOMNode* other){
    other->mName = mName;
    other->mRoomNumber = mRoomNumber;
	other->mGenType = mGenType;
	other->mPathName = mPathName;
	other->mCodeName = mCodeName;
	other->mSpawnFlag = mSpawnFlag;
	other->mDespawnFlag = mDespawnFlag;
	other->mActorSpawnRate = mActorSpawnRate;
	other->mActorsPerBurst = mActorsPerBurst;
	other->mActorSpawnLimit = mActorSpawnLimit;
	other->mArg0 = mArg0;
	other->mArg1 = mArg1;
	other->mArg2 = mArg2;
	other->mArg3 = mArg3;
	other->mArg4 = mArg4;
	other->mArg5 = mArg5;
	other->mArg6 = mArg6;
	other->mArg7 = mArg7;
	other->mArg8 = mArg8;
	other->mStay = mStay;

    other->mPosition = mPosition;
}

void LGeneratorDOMNode::RenderDetailsUI(float dt)
{
    LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

    // Strings
    if(LUIUtility::RenderTextInput("Type", &mGenType)){
        LEditorScene::GetEditorScene()->LoadActor(mGenType, false);
    }
    LUIUtility::RenderTooltip("What kind of actor this generator spawns.");

    LUIUtility::RenderTextInput("Path Name", &mPathName);
    LUIUtility::RenderTooltip("The name of a path file that this generator will use. Set this to (null) for no path.");

    LUIUtility::RenderTextInput("Script Name", &mCodeName);
    LUIUtility::RenderTooltip("The name that can be used to reference this generator in an event, via the <GENON> and <GENOFF> tags.");

    // Integers
    ImGui::InputInt("Spawn Flag", &mSpawnFlag);
    LUIUtility::RenderTooltip("The flag that must be set before this generator will begin spawning actors.");
    ImGui::InputInt("Despawn Flag", &mDespawnFlag);
    LUIUtility::RenderTooltip("If this flag is set, this generator will no longer spawn actors.");

    ImGui::InputInt("Burst Rate", &mActorSpawnRate);
    LUIUtility::RenderTooltip("How often the generator spawns actors, in frames.");
    ImGui::InputInt("Actors per Burst", &mActorsPerBurst);
    LUIUtility::RenderTooltip("The number of actors the generator spawns per burst.");
    ImGui::InputInt("Total Actor Limit", &mActorSpawnLimit);
    LUIUtility::RenderTooltip("The total number of actors that this generator can have active at one time.");

    ImGui::InputInt("Arg 0", &mArg0);
    LUIUtility::RenderTooltip("Arg0");
    ImGui::InputInt("Arg 1", &mArg1);
    LUIUtility::RenderTooltip("Arg1");
    ImGui::InputInt("Arg 2", &mArg2);
    LUIUtility::RenderTooltip("Arg2");
    ImGui::InputInt("Arg 3", &mArg3);
    LUIUtility::RenderTooltip("Arg3");
    ImGui::InputInt("Arg 4", &mArg4);
    LUIUtility::RenderTooltip("Arg4");
    ImGui::InputInt("Arg 5", &mArg5);
    LUIUtility::RenderTooltip("Arg5");
    ImGui::InputInt("Arg 6", &mArg6);
    LUIUtility::RenderTooltip("Arg6");
    ImGui::InputInt("Arg 7", &mArg7);
    LUIUtility::RenderTooltip("Arg7");
    ImGui::InputInt("Arg 8", &mArg8);
    LUIUtility::RenderTooltip("Arg8");

    // Bools
    LUIUtility::RenderCheckBox("Always Active", &mStay);
    LUIUtility::RenderTooltip("Keeps actor loaded regardless of current room");
}

void LGeneratorDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    JmpIO->SetString(entry_index, "name", mName);
    JmpIO->SetString(entry_index, "type", mGenType);
    JmpIO->SetString(entry_index, "path_name", mPathName);
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

    JmpIO->SetUnsignedInt(entry_index, "room_no", mRoomNumber);

    JmpIO->SetUnsignedInt(entry_index, "appear_flag", mSpawnFlag);
    JmpIO->SetUnsignedInt(entry_index, "disappear_flag", mDespawnFlag);

    JmpIO->SetUnsignedInt(entry_index, "appear_time", mActorSpawnRate);
    JmpIO->SetUnsignedInt(entry_index, "max_enemy_once", mActorsPerBurst);
    JmpIO->SetUnsignedInt(entry_index, "max_enemy", mActorSpawnLimit);

    JmpIO->SetUnsignedInt(entry_index, "arg0", mArg0);
    JmpIO->SetUnsignedInt(entry_index, "arg1", mArg1);
    JmpIO->SetUnsignedInt(entry_index, "arg2", mArg2);
    JmpIO->SetUnsignedInt(entry_index, "arg3", mArg3);
    JmpIO->SetUnsignedInt(entry_index, "arg4", mArg4);
    JmpIO->SetUnsignedInt(entry_index, "arg5", mArg5);
    JmpIO->SetUnsignedInt(entry_index, "arg6", mArg6);
    JmpIO->SetUnsignedInt(entry_index, "arg7", mArg7);
    JmpIO->SetUnsignedInt(entry_index, "arg8", mArg8);

    JmpIO->SetBoolean(entry_index, "stay", mStay);
}

void LGeneratorDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    mName = JmpIO->GetString(entry_index, "name");
    mGenType = JmpIO->GetString(entry_index, "type");
    mPathName = JmpIO->GetString(entry_index, "path_name");
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

    mRoomNumber = JmpIO->GetSignedInt(entry_index, "room_no");

    mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
    mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");

    mActorSpawnRate = JmpIO->GetSignedInt(entry_index, "appear_time");
    mActorsPerBurst = JmpIO->GetSignedInt(entry_index, "max_enemy_once");
    mActorSpawnLimit = JmpIO->GetSignedInt(entry_index, "max_enemy");

    mArg0 = JmpIO->GetSignedInt(entry_index, "arg0");
    mArg1 = JmpIO->GetSignedInt(entry_index, "arg1");
    mArg2 = JmpIO->GetSignedInt(entry_index, "arg2");
    mArg3 = JmpIO->GetSignedInt(entry_index, "arg3");
    mArg4 = JmpIO->GetSignedInt(entry_index, "arg4");
    mArg5 = JmpIO->GetSignedInt(entry_index, "arg5");
    mArg6 = JmpIO->GetSignedInt(entry_index, "arg6");
    mArg7 = JmpIO->GetSignedInt(entry_index, "arg7");
    mArg8 = JmpIO->GetSignedInt(entry_index, "arg8");

    mStay = JmpIO->GetBoolean(entry_index, "stay");
}

void LGeneratorDOMNode::PostProcess()
{
    /*
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();
    */
}

void LGeneratorDOMNode::PreProcess()
{
    /*
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();
    */
}