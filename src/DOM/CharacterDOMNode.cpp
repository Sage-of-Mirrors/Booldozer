#include "DOM/CharacterDOMNode.hpp"

LCharacterDOMNode::LCharacterDOMNode(std::string name) : Super(name),
	mCreateName("----"), mPathName("(null)"), mCodeName("(null)"),
	mSpawnFlag(0), mDespawnFlag(0), mEventNumber(0), mItemTable(0), mSpawnPointID(0), mGBHScanID(0),
	mCondType(0), mAttackType(0), mMoveType(0), mAppearType(0), mIsVisible(true), mStay(false)
{
	mType = EDOMNodeType::Character;
}

void LCharacterDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "create_name", mCreateName);
	JmpIO->SetString(entry_index, "path_name", mPathName);
	JmpIO->SetString(entry_index, "CodeName", mCodeName);

	JmpIO->SetFloat(entry_index, "pos_x", mPosition.x);
	JmpIO->SetFloat(entry_index, "pos_y", mPosition.y);
	JmpIO->SetFloat(entry_index, "pos_z", mPosition.z);

	glm::vec3 euler_angles = glm::eulerAngles(mRotation);
	JmpIO->SetFloat(entry_index, "dir_x", glm::degrees(euler_angles.x));
	JmpIO->SetFloat(entry_index, "dir_y", glm::degrees(euler_angles.y));
	JmpIO->SetFloat(entry_index, "dir_z", glm::degrees(euler_angles.z));

	JmpIO->SetSignedInt(entry_index, "room_no", mRoomNumber);

	JmpIO->SetSignedInt(entry_index, "appear_flag", mSpawnFlag);
	JmpIO->SetSignedInt(entry_index, "disappear_flag", mDespawnFlag);

	JmpIO->SetSignedInt(entry_index, "event_set_no", mEventNumber);
	JmpIO->SetSignedInt(entry_index, "item_table", mItemTable);
	JmpIO->SetSignedInt(entry_index, "appear_point", mSpawnPointID);
	JmpIO->SetSignedInt(entry_index, "msg_no", mGBHScanID);

	JmpIO->SetUnsignedInt(entry_index, "cond_type", mCondType);
	JmpIO->SetUnsignedInt(entry_index, "attack_type", mAttackType);
	JmpIO->SetUnsignedInt(entry_index, "move_type", mMoveType);
	JmpIO->SetUnsignedInt(entry_index, "appear_type", mAppearType);

	JmpIO->SetBoolean(entry_index, "invisible", mIsVisible);
	JmpIO->SetBoolean(entry_index, "stay", mStay);
}

void LCharacterDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mCreateName = JmpIO->GetString(entry_index, "create_name");
	mPathName = JmpIO->GetString(entry_index, "path_name");
	mCodeName = JmpIO->GetString(entry_index, "CodeName");

	mPosition.x = JmpIO->GetFloat(entry_index, "pos_x");
	mPosition.y = JmpIO->GetFloat(entry_index, "pos_y");
	mPosition.z = JmpIO->GetFloat(entry_index, "pos_z");

	float rotX = glm::radians(JmpIO->GetFloat(entry_index, "dir_x"));
	float rotY = glm::radians(JmpIO->GetFloat(entry_index, "dir_y"));
	float rotZ = glm::radians(JmpIO->GetFloat(entry_index, "dir_z"));
	mRotation = glm::quat(glm::vec3(rotX, rotY, rotZ));

	mRoomNumber = JmpIO->GetSignedInt(entry_index, "room_no");

	mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
	mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");

	mEventNumber = JmpIO->GetSignedInt(entry_index, "event_set_no");
	mItemTable = JmpIO->GetSignedInt(entry_index, "item_table");
	mSpawnPointID = JmpIO->GetSignedInt(entry_index, "appear_point");
	mGBHScanID = JmpIO->GetSignedInt(entry_index, "msg_no");

	mCondType = JmpIO->GetUnsignedInt(entry_index, "cond_type");
	mAttackType = JmpIO->GetUnsignedInt(entry_index, "attack_type");
	mMoveType = JmpIO->GetUnsignedInt(entry_index, "move_type");
	mAppearType = JmpIO->GetUnsignedInt(entry_index, "appear_type");

	mIsVisible = JmpIO->GetBoolean(entry_index, "invisible");
	mStay = JmpIO->GetBoolean(entry_index, "stay");
}
