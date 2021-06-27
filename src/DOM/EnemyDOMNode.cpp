#include "DOM/EnemyDOMNode.hpp"
#include "UIUtil.hpp"

std::map<std::string, std::string> EnemyNames = {
	{ "yapoo1", "Gold Ghost" },
	{ "yapoo2", "Temper Terror" },

	{ "mapoo1", "Purple Basher" },
	{ "mapoo2", "Flash" },

	{ "mopoo1", "Blue Twirler" },
	{ "mopoo2", "Blue Blaze" },

	{ "banaoba", "Garbage Can Ghost" },

	{ "topoo1", "Red Grabbing Ghost" },
	{ "topoo2", "Turqoise Grabbing Ghost" },
	{ "topoo3", "Purple Grabbing Ghost" },
	{ "topoo4", "White Grabbing Ghost" },

	{ "putcher1", "Bowling Ghost" },

	{ "iyapoo1", "Speedy Spirit 1" },
	{ "iyapoo2", "Speedy Spirit 2" },
	{ "iyapoo3", "Speedy Spirit 3" },
	{ "iyapoo4", "Speedy Spirit 4" },
	{ "iyapoo5", "Speedy Spirit 5" },
	{ "iyapoo6", "Speedy Spirit 6" },
	{ "iyapoo7", "Speedy Spirit 7" },
	{ "iyapoo8", "Speedy Spirit 8" },
	{ "iyapoo9", "Speedy Spirit 9" },
	{ "iyapoo10", "Speedy Spirit 10" },
	{ "iyapoo11", "Speedy Spirit 11" },
	{ "iyapoo12", "Speedy Spirit 12" },

	{ "piero1", "Red Clown Doll" },
	{ "piero2", "Blue Clown Doll" },

	{ "heypo1", "Red Shy Guy Ghost" },
	{ "heypo2", "Green Shy Guy Ghost" },
	{ "heypo3", "White Shy Guy Ghost" },
	{ "heypo4", "Orange Shy Guy Ghost" },
	{ "heypo5", "Orange Shy Guy Ghost?" },
	{ "heypo6", "Yellow Shy Guy Ghost" },
	{ "heypo7", "Pink Shy Guy Ghost" },
	{ "heypo8", "Purple Shy Guy Ghost" },

	{ "tenjyo", "Purple Bomber" },
	{ "tenjyo2", "Ceiling Surprise" },

	{ "fakedoor", "Fake Square Door" },
	{ "fkdoor2", "Fake Wooden Door" },
	{ "fkdoor3", "Fake Round Door" },

	{ "ifly", "Frying Pan" },
	{ "inabe", "Pot" },
	{ "ibookr", "Book" },
	{ "polter1", "polter1" },

	{ "skul", "Mr. Bones" },

	{ "otub1", "Blue Vase" },
	{ "otub2", "Grey Vase" },
	{ "otub3", "Pink/White Vase" },
	{ "otub4", "Red/Blue/White Vase" },
	{ "otub5", "Blue/White Vase" },

	{ "oufo1", "Plate" },
	{ "oufo2", "Bowl" },
};

LEnemyDOMNode::LEnemyDOMNode(std::string name) : Super(name),
	mCreateName("----"), mPathName("(null)"), mAccessName("(null)"), mCodeName("(null)"),
	mFloatingHeight(0), mAppearChance(64), mSpawnFlag(0), mDespawnFlag(0), mEventNumber(0), mItemTable(0),
	mCondType(0), mMoveType(0), mSearchType(0), mAppearType(0), mPlaceType(0), mIsVisible(true), mStay(false)
{
	mType = EDOMNodeType::Enemy;
}

void LEnemyDOMNode::RenderDetailsUI(float dt)
{
	LUIUtility::RenderComboBox("Enemy Type", EnemyNames, mName);
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

	JmpIO->SetFloat(entry_index, "dir_x", mRotation.x);
	JmpIO->SetFloat(entry_index, "dir_y", mRotation.y);
	JmpIO->SetFloat(entry_index, "dir_z", mRotation.z);

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

	mRotation.x = JmpIO->GetFloat(entry_index, "dir_x");
	mRotation.y = JmpIO->GetFloat(entry_index, "dir_y");
	mRotation.z = JmpIO->GetFloat(entry_index, "dir_z");

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
