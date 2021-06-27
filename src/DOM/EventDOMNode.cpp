#include "DOM/EventDOMNode.hpp"
#include "imgui.h"
#include "imgui_internal.h"

LEventDOMNode::LEventDOMNode(std::string name) : Super(name),
	mCharacterName("(null)"), mEventNo(0), mActivationRadius(0), mEventFlag(0),
	mMinHour(0), mMinMinute(0), mMaxHour(0), mMaxMinute(0), mMaxTriggerCount(0),
	mDespawnFlag(0), mParameter(0), mEventIf(0), mCanBeInterrupted(false), mFreezePlayer(false)
{
	mType = EDOMNodeType::Event;
	mRoomNumber = -1;
}

//void LEventDOMNode::RenderHierarchyUI(float dt)
//{
//	char evtNo[16];
//	snprintf(evtNo, 16, "Event %02d", mEventNo);
//}

void LEventDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "CharacterName", mCharacterName);

	JmpIO->SetFloat(entry_index, "pos_x", mPosition.x);
	JmpIO->SetFloat(entry_index, "pos_y", mPosition.y);
	JmpIO->SetFloat(entry_index, "pos_z", mPosition.z);

	JmpIO->SetFloat(entry_index, "dir_x", mRotation.x);
	JmpIO->SetFloat(entry_index, "dir_y", mRotation.y);
	JmpIO->SetFloat(entry_index, "dir_z", mRotation.z);

	JmpIO->SetFloat(entry_index, "scale_x", mScale.x);
	JmpIO->SetFloat(entry_index, "scale_y", mScale.y);
	JmpIO->SetFloat(entry_index, "scale_z", mScale.z);

	JmpIO->SetSignedInt(entry_index, "EventNo", mEventNo);
	JmpIO->SetSignedInt(entry_index, "EventArea", mActivationRadius);
	JmpIO->SetSignedInt(entry_index, "EventFlag", mEventFlag);

	JmpIO->SetSignedInt(entry_index, "EventTime", mMinHour);
	JmpIO->SetSignedInt(entry_index, "EventTime2", mMinMinute);
	JmpIO->SetSignedInt(entry_index, "EventTime3", mMaxHour);
	JmpIO->SetSignedInt(entry_index, "EventTime4", mMaxMinute);

	JmpIO->SetSignedInt(entry_index, "EventLoad", mMaxTriggerCount);
	JmpIO->SetSignedInt(entry_index, "disappear_flag", mDespawnFlag);
	JmpIO->SetSignedInt(entry_index, "event_parameter", mParameter);

	JmpIO->SetUnsignedInt(entry_index, "EventIf", mEventIf);

	JmpIO->SetBoolean(entry_index, "EventLock", mCanBeInterrupted);
	JmpIO->SetBoolean(entry_index, "PlayerStop", mFreezePlayer);
}

void LEventDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mCharacterName = JmpIO->GetString(entry_index, "CharacterName");

	mPosition.x = JmpIO->GetFloat(entry_index, "pos_x");
	mPosition.y = JmpIO->GetFloat(entry_index, "pos_y");
	mPosition.z = JmpIO->GetFloat(entry_index, "pos_z");

	mRotation.x = JmpIO->GetFloat(entry_index, "dir_x");
	mRotation.y = JmpIO->GetFloat(entry_index, "dir_y");
	mRotation.z = JmpIO->GetFloat(entry_index, "dir_z");

	mScale.x = JmpIO->GetFloat(entry_index, "scale_x");
	mScale.y = JmpIO->GetFloat(entry_index, "scale_y");
	mScale.z = JmpIO->GetFloat(entry_index, "scale_z");

	mEventNo = JmpIO->GetSignedInt(entry_index, "EventNo");
	mActivationRadius = JmpIO->GetSignedInt(entry_index, "EventArea");
	mEventFlag = JmpIO->GetSignedInt(entry_index, "EventFlag");

	mMinHour = JmpIO->GetSignedInt(entry_index, "EventTime");
	mMinMinute = JmpIO->GetSignedInt(entry_index, "EventTime2");
	mMaxHour = JmpIO->GetSignedInt(entry_index, "EventTime3");
	mMaxMinute = JmpIO->GetSignedInt(entry_index, "EventTime4");

	mMaxTriggerCount = JmpIO->GetSignedInt(entry_index, "EventLoad");

	mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");
	mParameter = JmpIO->GetSignedInt(entry_index, "event_parameter");

	mEventIf = JmpIO->GetUnsignedInt(entry_index, "EventIf");

	mCanBeInterrupted = JmpIO->GetBoolean(entry_index, "EventLock");
	mFreezePlayer = JmpIO->GetBoolean(entry_index, "PlayerStop");
}