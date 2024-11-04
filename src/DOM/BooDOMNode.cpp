#include "DOM/BooDOMNode.hpp"
#include "UIUtil.hpp"

LBooDOMNode::LBooDOMNode(std::string name) : Super(name),
    mInitialRoom(0), mNextRoomWait(0), mAcceleration(0.f), mMaxSpeed(0.f), mAngle(0.f),
    mHP(0), mMoveTime(0), mAttacks(false)
{
    mType = EDOMNodeType::Boo;
    mRoomNumber = -1;
}

void LBooDOMNode::RenderDetailsUI(float dt)
{
    // Integers
    if(ImGui::InputInt("Initial Room", &mInitialRoom)){
        mName = std::format("Boo {}", mInitialRoom);
    }
    
    LUIUtility::RenderTooltip("The room that this Boo spawns in.");

    ImGui::InputInt("Next Room Wait", &mNextRoomWait);
    LUIUtility::RenderTooltip("?");

    ImGui::InputInt("HP", &mHP);
    LUIUtility::RenderTooltip("How much health the Boo has.");

    ImGui::InputInt("Move Time", &mMoveTime);
    LUIUtility::RenderTooltip("How long the Boo should wait before leaving the room.");

    // Floats
    ImGui::InputFloat("Acceleration", &mAcceleration);
    LUIUtility::RenderTooltip("How quickly the Boo accelerates towards the next point in its path. Ensure this is smaller than Max Speed.");

    ImGui::InputFloat("Max Speed", &mMaxSpeed);
    LUIUtility::RenderTooltip("The limit on how fast the Boo can move. Ensure this is larger than Acceleration.");

    ImGui::InputFloat("Angle", &mAngle);
    LUIUtility::RenderTooltip("?");

    // Bools
    LUIUtility::RenderCheckBox("Attacks", &mAttacks);
    LUIUtility::RenderTooltip("Whether the Boo should dive-bomb Luigi.");
}

void LBooDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    JmpIO->SetUnsignedInt(entry_index, "init_room", mInitialRoom);
    JmpIO->SetUnsignedInt(entry_index, "next_room_wait", mNextRoomWait);

    JmpIO->SetFloat(entry_index, "accel", mAcceleration);
    JmpIO->SetFloat(entry_index, "max_speed", mMaxSpeed);
    JmpIO->SetFloat(entry_index, "rnd_angle", mAngle);

    JmpIO->SetUnsignedInt(entry_index, "str_hp", mHP);
    JmpIO->SetUnsignedInt(entry_index, "move_time", mMoveTime);

    JmpIO->SetBoolean(entry_index, "attack", mAttacks);
}

void LBooDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    mInitialRoom = JmpIO->GetSignedInt(entry_index, "init_room");
    mNextRoomWait = JmpIO->GetSignedInt(entry_index, "next_room_wait");

    mAcceleration = JmpIO->GetFloat(entry_index, "accel");
    mMaxSpeed = JmpIO->GetFloat(entry_index, "max_speed");
    mAngle = JmpIO->GetFloat(entry_index, "rnd_angle");

    mHP = JmpIO->GetSignedInt(entry_index, "str_hp");
    mMoveTime = JmpIO->GetSignedInt(entry_index, "move_time");

    mAttacks = JmpIO->GetBoolean(entry_index, "attack");
}

void LBooDOMNode::PostProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();

    mName = std::format("Boo {}", mInitialRoom);

}

void LBooDOMNode::PreProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();
}
