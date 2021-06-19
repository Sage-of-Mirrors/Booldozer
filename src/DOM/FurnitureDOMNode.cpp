#include "DOM/FurnitureDOMNode.hpp"

LFurnitureDOMNode::LFurnitureDOMNode(std::string name) : LEntityDOMNode(name),
	mInternalName("(null)"), mModelName("(null)"), mAccessName("(null)"),
	mVerticalItemSpawnOffset(0.0f), mItemTableIndex(-1), mGenerateNumber(-1), mBooHideChance(-1),
	mShakeIntensity(-1), mVecArgs(glm::vec3(0.0f, 0.0f, 0.0f)), mSpawnFlag(-1), mDespawnFlag(-1),
	mHitboxExtents(glm::ivec3(0, 0, 0)), mGBHScanID(-1), mBehaviorType(0), mSoundID(0), mSheetBehavior(0),
	mMoneyType(0), mSheetTexture(0), mShouldCutaway(false), mCanSheetBeVaccuumed(false), mBooAppear(false)
{
	mType = EDOMNodeType::Furniture;
}

void LFurnitureDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mInternalName);
	JmpIO->SetString(entry_index, "dmd_name", mModelName);
	JmpIO->SetString(entry_index, "access_name", mAccessName);

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

	JmpIO->SetFloat(entry_index, "item_offset_y", mVerticalItemSpawnOffset);

	JmpIO->SetSignedInt(entry_index, "item_table", mItemTableIndex);
	JmpIO->SetSignedInt(entry_index, "generate_num", mGenerateNumber);
	JmpIO->SetSignedInt(entry_index, "telesa_hide", mBooHideChance);
	JmpIO->SetSignedInt(entry_index, "move_level", mShakeIntensity);

	JmpIO->SetFloat(entry_index, "arg0", mVecArgs.x);
	JmpIO->SetFloat(entry_index, "arg1", mVecArgs.y);
	JmpIO->SetFloat(entry_index, "arg2", mVecArgs.z);

	JmpIO->SetSignedInt(entry_index, "appear_flag", mSpawnFlag);
	JmpIO->SetSignedInt(entry_index, "disappear_flag", mDespawnFlag);

	JmpIO->SetSignedInt(entry_index, "furniture_x", mHitboxExtents.x);
	JmpIO->SetSignedInt(entry_index, "furniture_y", mHitboxExtents.y);
	JmpIO->SetSignedInt(entry_index, "furniture_z", mHitboxExtents.z);

	JmpIO->SetSignedInt(entry_index, "counter", mGBHScanID);

	JmpIO->SetUnsignedInt(entry_index, "move", mBehaviorType);
	JmpIO->SetUnsignedInt(entry_index, "furniture_sound", mSoundID);
	JmpIO->SetUnsignedInt(entry_index, "sheet", mSheetBehavior);
	JmpIO->SetUnsignedInt(entry_index, "generate", mMoneyType);
	JmpIO->SetUnsignedInt(entry_index, "sheet_texture", mSheetTexture);

	JmpIO->SetBoolean(entry_index, "not_transparent", mShouldCutaway);
	JmpIO->SetBoolean(entry_index, "sheet_gum", mCanSheetBeVaccuumed);
	JmpIO->SetBoolean(entry_index, "telesa_appear", mBooAppear);
}

void LFurnitureDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mInternalName = JmpIO->GetString(entry_index, "name");
	mModelName = JmpIO->GetString(entry_index, "dmd_name");
	mAccessName = JmpIO->GetString(entry_index, "access_name");

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

	mVerticalItemSpawnOffset = JmpIO->GetFloat(entry_index, "item_offset_y");

	mItemTableIndex = JmpIO->GetSignedInt(entry_index, "item_table");
	mGenerateNumber = JmpIO->GetSignedInt(entry_index, "generate_num");
	mBooHideChance = JmpIO->GetSignedInt(entry_index, "telesa_hide");
	mShakeIntensity = JmpIO->GetSignedInt(entry_index, "move_level");

	mVecArgs.x = JmpIO->GetFloat(entry_index, "arg0");
	mVecArgs.y = JmpIO->GetFloat(entry_index, "arg1");
	mVecArgs.z = JmpIO->GetFloat(entry_index, "arg2");

	mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
	mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");

	mHitboxExtents.x = JmpIO->GetSignedInt(entry_index, "furniture_x");
	mHitboxExtents.y = JmpIO->GetSignedInt(entry_index, "furniture_y");
	mHitboxExtents.z = JmpIO->GetSignedInt(entry_index, "furniture_z");

	mGBHScanID = JmpIO->GetSignedInt(entry_index, "counter");

	mBehaviorType = JmpIO->GetUnsignedInt(entry_index, "move");
	mSoundID = JmpIO->GetUnsignedInt(entry_index, "furniture_sound");
	mSheetBehavior = JmpIO->GetUnsignedInt(entry_index, "sheet");
	mMoneyType = JmpIO->GetUnsignedInt(entry_index, "generate");
	mSheetTexture = JmpIO->GetUnsignedInt(entry_index, "sheet_texture");

	mShouldCutaway = JmpIO->GetBoolean(entry_index, "not_transparent");
	mCanSheetBeVaccuumed = JmpIO->GetBoolean(entry_index, "sheet_gum");
	mBooAppear = JmpIO->GetBoolean(entry_index, "telesa_appear");
}
