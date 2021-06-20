#include "DOM/ObserverDOMNode.hpp"

LObserverDOMNode::LObserverDOMNode(std::string name) : Super(name),
	mCodeName("(null)"), mCondStringArg0("(null)"), mStringArg0("(null)"),
	mCondArg0(0), mArg0(0), mArg1(0), mArg2(0), mArg3(0), mArg4(0), mArg5(0), mSpawnFlag(0), mDespawnFlag(0),
	mCondType(0), mDoType(0), mIsVisible(false), mUnkBool1(false)
{
	mType = EDOMNodeType::Observer;
}

void LObserverDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "CodeName", mCodeName);
	JmpIO->SetString(entry_index, "cond_string_arg0", mCondStringArg0);
	JmpIO->SetString(entry_index, "string_arg0", mStringArg0);

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

	JmpIO->SetSignedInt(entry_index, "cond_arg0", mCondArg0);

	JmpIO->SetSignedInt(entry_index, "arg0", mArg0);
	JmpIO->SetSignedInt(entry_index, "arg1", mArg1);
	JmpIO->SetSignedInt(entry_index, "arg2", mArg2);
	JmpIO->SetSignedInt(entry_index, "arg3", mArg3);
	JmpIO->SetSignedInt(entry_index, "arg4", mArg4);
	JmpIO->SetSignedInt(entry_index, "arg5", mArg5);

	JmpIO->SetSignedInt(entry_index, "appear_flag", mSpawnFlag);
	JmpIO->SetSignedInt(entry_index, "disappear_flag", mDespawnFlag);

	JmpIO->SetUnsignedInt(entry_index, "cond_type", mCondType);
	JmpIO->SetUnsignedInt(entry_index, "do_type", mDoType);

	JmpIO->SetBoolean(entry_index, "invisible", mIsVisible);
}

void LObserverDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mCodeName = JmpIO->GetString(entry_index, "CodeName");
	mCondStringArg0 = JmpIO->GetString(entry_index, "cond_string_arg0");
	mStringArg0 = JmpIO->GetString(entry_index, "string_arg0");

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

	mCondArg0 = JmpIO->GetSignedInt(entry_index, "cond_arg0");

	mArg0 = JmpIO->GetSignedInt(entry_index, "arg0");
	mArg1 = JmpIO->GetSignedInt(entry_index, "arg1");
	mArg2 = JmpIO->GetSignedInt(entry_index, "arg2");
	mArg3 = JmpIO->GetSignedInt(entry_index, "arg3");
	mArg4 = JmpIO->GetSignedInt(entry_index, "arg4");
	mArg5 = JmpIO->GetSignedInt(entry_index, "arg5");

	mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
	mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");

	mCondType = JmpIO->GetUnsignedInt(entry_index, "cond_type");
	mDoType = JmpIO->GetUnsignedInt(entry_index, "do_type");

	mIsVisible = JmpIO->GetBoolean(entry_index, "invisible");
}
