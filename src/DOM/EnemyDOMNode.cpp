#include "DOM/EnemyDOMNode.hpp"

LEnemyDOMNode::LEnemyDOMNode(std::string name) : Super(name),
	mCreateName("----"), mPathName("(null)"), mAccessName("(null)"), mCodeName("(null)"),
	mFloatingHeight(0), mAppearChance(64), mSpawnFlag(0), mDespawnFlag(0), mEventNumber(0), mItemTable(0),
	mCondType(0), mMoveType(0), mSearchType(0), mAppearType(0), mPlaceType(0), mIsVisible(true), mStay(false)
{
	mType = EDOMNodeType::Enemy;
}

void LEnemyDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "create_name", mCreateName);
	JmpIO->SetString(entry_index, "path_name", mPathName);
	JmpIO->SetString(entry_index, "access_name", mAccessName);
	JmpIO->SetString(entry_index, "CodeName", mCodeName);

	JmpIO->SetFloat(entry_index, "pos_x", mPosition.x);
	JmpIO->SetFloat(entry_index, "pos_y", mPosition.y);
	JmpIO->SetFloat(entry_index, "pos_z", mPosition.z);

	glm::vec3 euler_angles = glm::eulerAngles(mRotation);
	JmpIO->SetFloat(entry_index, "dir_x", glm::degrees(euler_angles.x));
	JmpIO->SetFloat(entry_index, "dir_y", glm::degrees(euler_angles.y));
	JmpIO->SetFloat(entry_index, "dir_z", glm::degrees(euler_angles.z));

	JmpIO->SetFloat(entry_index, "scale_x", mScale.x);
	JmpIO->SetFloat(entry_index, "scale_y", mScale.y);
	JmpIO->SetFloat(entry_index, "scale_z", mScale.z);

	JmpIO->SetSignedInt(entry_index, "room_no", mRoomNumber);

	JmpIO->SetSignedInt(entry_index, "floating_height", mFloatingHeight);
	JmpIO->SetSignedInt(entry_index, "appear_percent", mAppearChance);

	JmpIO->SetSignedInt(entry_index, "appear_flag", mSpawnFlag);
	JmpIO->SetSignedInt(entry_index, "disappear_flag", mDespawnFlag);

	JmpIO->SetSignedInt(entry_index, "event_set_no", mEventNumber);
	JmpIO->SetSignedInt(entry_index, "item_table", mItemTable);

	JmpIO->SetUnsignedInt(entry_index, "cond_type", mCondType);
	JmpIO->SetUnsignedInt(entry_index, "move_type", mMoveType);
	JmpIO->SetUnsignedInt(entry_index, "search_type", mSearchType);
	JmpIO->SetUnsignedInt(entry_index, "appear_type", mAppearType);
	JmpIO->SetUnsignedInt(entry_index, "place_type", mPlaceType);

	JmpIO->SetBoolean(entry_index, "invisible", mIsVisible);
	JmpIO->SetBoolean(entry_index, "stay", mStay);
}

void LEnemyDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mCreateName = JmpIO->GetString(entry_index, "create_name");
	mPathName = JmpIO->GetString(entry_index, "path_name");
	mAccessName = JmpIO->GetString(entry_index, "access_name");
	mCodeName = JmpIO->GetString(entry_index, "CodeName");

	mPosition.x = JmpIO->GetFloat(entry_index, "pos_x");
	mPosition.y = JmpIO->GetFloat(entry_index, "pos_y");
	mPosition.z = JmpIO->GetFloat(entry_index, "pos_z");

	float rotX = glm::radians(JmpIO->GetFloat(entry_index, "dir_x"));
	float rotY = glm::radians(JmpIO->GetFloat(entry_index, "dir_y"));
	float rotZ = glm::radians(JmpIO->GetFloat(entry_index, "dir_z"));
	mRotation = glm::quat(glm::vec3(rotX, rotY, rotZ));

	mScale.x = JmpIO->GetFloat(entry_index, "scale_x");
	mScale.y = JmpIO->GetFloat(entry_index, "scale_y");
	mScale.z = JmpIO->GetFloat(entry_index, "scale_z");

	mRoomNumber = JmpIO->GetSignedInt(entry_index, "room_no");

	mFloatingHeight = JmpIO->GetSignedInt(entry_index, "floating_height");
	mAppearChance = JmpIO->GetSignedInt(entry_index, "appear_percent");

	mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
	mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");

	mEventNumber = JmpIO->GetSignedInt(entry_index, "event_set_no");
	mItemTable = JmpIO->GetSignedInt(entry_index, "item_table");

	mCondType = JmpIO->GetUnsignedInt(entry_index, "cond_type");
	mMoveType = JmpIO->GetUnsignedInt(entry_index, "move_type");
	mSearchType = JmpIO->GetUnsignedInt(entry_index, "search_type");
	mAppearType = JmpIO->GetUnsignedInt(entry_index, "appear_type");
	mPlaceType = JmpIO->GetUnsignedInt(entry_index, "place_type");

	mIsVisible = JmpIO->GetBoolean(entry_index, "invisible");
	mStay = JmpIO->GetBoolean(entry_index, "stay");
}
