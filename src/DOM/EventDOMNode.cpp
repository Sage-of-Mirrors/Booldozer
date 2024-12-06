#include "DOM/EventDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "DOM/RoomDataDOMNode.hpp"
#include "UIUtil.hpp"
#include "imgui.h"

LEventDOMNode::LEventDOMNode(std::string name) : Super(name),
	mCharacterName("(null)"), mEventNo(0), mActivationRadius(0), mSpawnFlag(0),
	mMinHour(0), mMinMinute(0), mMaxHour(0), mMaxMinute(0), mMaxTriggerCount(0),
	mDespawnFlag(0), mParameter(0), mEventIf(EEventIfType::Repeatedly_from_Anywhere), mCanBeInterrupted(false), mFreezePlayer(false)
{
	mType = EDOMNodeType::Event;
	mRoomNumber = -1;
}

void LEventDOMNode::RenderDetailsUI(float dt)
{
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

	// Strings
	LUIUtility::RenderTextInput("Character Name", &mCharacterName);
	LUIUtility::RenderTooltip("The name of an actor. If that actor is alive and its Associated Event Index matches this event, the event will be allowed to trigger. Otherwise, it will not be triggered.");

	// Integers
	ImGui::InputInt("Trigger Radius", &mActivationRadius);
	LUIUtility::RenderTooltip("Determines the radius of an infinitely tall cylinder or a sphere, depending on the Event Trigger Type, which will be used to determine if Luigi can trigger the event.");

	ImGui::InputInt("Event Archive Number", &mEventNo);
	LUIUtility::RenderTooltip("The archive from the Event directory to load for this event. The archive is named event[number].szp.");

	ImGui::InputInt("Spawn Flag", &mSpawnFlag);
	LUIUtility::RenderTooltip("The flag that must be set before this event will be allowed to trigger.");
	ImGui::InputInt("Despawn Flag", &mDespawnFlag);
	LUIUtility::RenderTooltip("If this flag is set, this event will no longer be allowed to trigger.");

	ImGui::InputInt("Maximum Trigger Count", &mMaxTriggerCount);
	LUIUtility::RenderTooltip("How many times this event will be allowed to trigger, before being disabled permanently. If set to 0, the event will never be disabled.");

	ImGui::InputInt("Gallery Portrait ID", &mParameter);
	LUIUtility::RenderTooltip("Which portrait to display when the <GALLERY> event tag is used.");

	// Ranges
	if (ImGui::DragIntRange2("Hour Range", &mMinHour, &mMaxHour, 1, 0, HOUR_MAX, "Min Hour: %d", "Max Hour: %d"))
	{
		if (mMinHour < 0)
			mMinHour = 0;
		if (mMinHour > HOUR_MAX)
			mMinHour = HOUR_MAX;

		if (mMaxHour < 0)
			mMaxHour = 0;
		if (mMaxHour > HOUR_MAX)
			mMaxHour = HOUR_MAX;
	}
	LUIUtility::RenderTooltip("UNUSED: This event will only be allowed to trigger between these hours on the GameBoy Horror clock.");

	if (ImGui::DragIntRange2("Minute Range", &mMinMinute, &mMaxMinute, 1, 0, MINUTE_MAX, "Min Minute: %d", "Max Minute: %d"))
	{
		if (mMinMinute < 0)
			mMinMinute = 0;
		if (mMinMinute > MINUTE_MAX)
			mMinMinute = MINUTE_MAX;

		if (mMaxMinute < 0)
			mMaxMinute = 0;
		if (mMaxMinute > MINUTE_MAX)
			mMaxMinute = MINUTE_MAX;
	}
	LUIUtility::RenderTooltip("UNUSED: This event will only be allowed to trigger between these minutes in the above hour range on the GameBoy Horror clock.");

	// Comboboxes
	LUIUtility::RenderComboEnum<EEventIfType>("Event Trigger Type", mEventIf);
	LUIUtility::RenderTooltip("How Luigi must interact with the event to trigger it.");

	// Bools
	LUIUtility::RenderCheckBox("Disable Interruption", &mCanBeInterrupted);
	LUIUtility::RenderTooltip("If checked, this event cannot be interrupted by another event triggering while this event is playing.");

	LUIUtility::RenderCheckBox("Freeze Luigi", &mFreezePlayer);
	LUIUtility::RenderTooltip("Whether this event should freeze Luigi while it is loading.");
}

void LEventDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "CharacterName", mCharacterName);

	JmpIO->SetFloat(entry_index, "pos_x", mPosition.z);
	JmpIO->SetFloat(entry_index, "pos_y", mPosition.y);
	JmpIO->SetFloat(entry_index, "pos_z", mPosition.x);

	JmpIO->SetFloat(entry_index, "dir_x", mRotation.z);
	JmpIO->SetFloat(entry_index, "dir_y", -mRotation.y);
	JmpIO->SetFloat(entry_index, "dir_z", mRotation.x);

	JmpIO->SetFloat(entry_index, "scale_x", mScale.z);
	JmpIO->SetFloat(entry_index, "scale_y", mScale.y);
	JmpIO->SetFloat(entry_index, "scale_z", mScale.x);

	JmpIO->SetUnsignedInt(entry_index, "EventNo", mEventNo);
	JmpIO->SetUnsignedInt(entry_index, "EventArea", mActivationRadius);
	JmpIO->SetUnsignedInt(entry_index, "EventFlag", mSpawnFlag);

	JmpIO->SetUnsignedInt(entry_index, "EventTime", mMinHour);
	JmpIO->SetUnsignedInt(entry_index, "EventTime2", mMinMinute);
	JmpIO->SetUnsignedInt(entry_index, "EventTime3", mMaxHour);
	JmpIO->SetUnsignedInt(entry_index, "EventTime4", mMaxMinute);

	JmpIO->SetUnsignedInt(entry_index, "EventLoad", mMaxTriggerCount);
	JmpIO->SetUnsignedInt(entry_index, "disappear_flag", mDespawnFlag);
	JmpIO->SetUnsignedInt(entry_index, "event_parameter", mParameter);

	JmpIO->SetUnsignedInt(entry_index, "EventIf", (uint32_t)mEventIf);

	JmpIO->SetBoolean(entry_index, "EventLock", mCanBeInterrupted);
	JmpIO->SetBoolean(entry_index, "PlayerStop", mFreezePlayer);
}

void LEventDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mCharacterName = JmpIO->GetString(entry_index, "CharacterName");

	mPosition.z = JmpIO->GetFloat(entry_index, "pos_x");
	mPosition.y = JmpIO->GetFloat(entry_index, "pos_y");
	mPosition.x = JmpIO->GetFloat(entry_index, "pos_z");

	mRotation.z = JmpIO->GetFloat(entry_index, "dir_x");
	mRotation.y = -JmpIO->GetFloat(entry_index, "dir_y");
	mRotation.x = JmpIO->GetFloat(entry_index, "dir_z");

	mScale.z = JmpIO->GetFloat(entry_index, "scale_x");
	mScale.y = JmpIO->GetFloat(entry_index, "scale_y");
	mScale.x = JmpIO->GetFloat(entry_index, "scale_z");

	mEventNo = JmpIO->GetSignedInt(entry_index, "EventNo");
	mActivationRadius = JmpIO->GetSignedInt(entry_index, "EventArea");
	mSpawnFlag = JmpIO->GetSignedInt(entry_index, "EventFlag");

	mMinHour = JmpIO->GetSignedInt(entry_index, "EventTime");
	mMinMinute = JmpIO->GetSignedInt(entry_index, "EventTime2");
	mMaxHour = JmpIO->GetSignedInt(entry_index, "EventTime3");
	mMaxMinute = JmpIO->GetSignedInt(entry_index, "EventTime4");

	mMaxTriggerCount = JmpIO->GetSignedInt(entry_index, "EventLoad");

	mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");
	mParameter = JmpIO->GetSignedInt(entry_index, "event_parameter");

	mEventIf = (EEventIfType)JmpIO->GetUnsignedInt(entry_index, "EventIf");

	mCanBeInterrupted = JmpIO->GetBoolean(entry_index, "EventLock");
	mFreezePlayer = JmpIO->GetBoolean(entry_index, "PlayerStop");
}

void LEventDOMNode::PostProcess()
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
			//LGenUtility::Log << LGenUtility::Format("Event ", mEventNo, " going to ", containingRoom->GetName(), mEventIf == EEventIfType::Repeatedly_from_Anywhere ? " (Anywhere)" : " (In range)") << std::endl;
			containingRoom->AddChild(GetSharedPtr<LEventDOMNode>(EDOMNodeType::Event));
		}
	}
}

void LEventDOMNode::PreProcess()
{

}
