#include "DOM/CharacterDOMNode.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "DOM/ItemAppearDOMNode.hpp"
#include "UIUtil.hpp"

LCharacterDOMNode::LCharacterDOMNode(std::string name) : LCharacterDOMNode(name, false)
{

}

LCharacterDOMNode::LCharacterDOMNode(std::string name, bool isBlackoutCharacter) : Super(name, isBlackoutCharacter),
	mCreateName("----"), mPathName("(null)"), mCodeName("(null)"),
	mSpawnFlag(0), mDespawnFlag(0), mEventSetNumber(0), mItemTableIndex(0), mSpawnPointID(0), mGBHScanID(0),
	mCondType(EConditionType::Always_True), mAttackType(EAttackType::None), mMoveType(0), mAppearType(EAppearType::Normal), mIsVisible(true), mStay(false),
	mItemTableRef(std::weak_ptr<LItemAppearDOMNode>())
{
	mType = EDOMNodeType::Character;
}

void LCharacterDOMNode::RenderDetailsUI(float dt)
{
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

	// Strings
	if(LUIUtility::RenderTextInput("Character Name", &mName)){
		LEditorScene::GetEditorScene()->LoadActor(mName, false);
	}
	LUIUtility::RenderTooltip("What character this entity is.");

	LUIUtility::RenderTextInput("Spawn Group", &mCreateName);
	LUIUtility::RenderTooltip("What Spawn Group this character is in. Set this to ---- to spawn when Luigi enters the room. This is also known as create_name.");

	LUIUtility::RenderTextInput("Path Name", &mPathName);
	LUIUtility::RenderTooltip("The name of a path file that this character will use. Set this to (null) for no path.");

	LUIUtility::RenderTextInput("Script Name", &mCodeName);
	LUIUtility::RenderTooltip("The name that can be used to reference this character in an event.");

	// Integers
	ImGui::InputInt("Spawn Flag", &mSpawnFlag);
	LUIUtility::RenderTooltip("The flag that must be set before this character will begin spawning.");
	ImGui::InputInt("Despawn Flag", &mDespawnFlag);
	LUIUtility::RenderTooltip("If this flag is set, this character will no longer spawn.");

	ImGui::InputInt("event_set_no", &mEventSetNumber);
	LUIUtility::RenderTooltip("Don't quite know what this does. It's called event_set_no by the game, but it seems useless?");

	if (mName == "luige")
	{
		ImGui::InputInt("Luigi Spawn ID", &mSpawnPointID);
		LUIUtility::RenderTooltip("The ID of this Luigi spawn point. Luigi will spawn into a map by default at ID 0; any other IDs must be accessed via events.");
	}

	ImGui::InputInt("GameBoy Horror Scan ID", &mGBHScanID);
	LUIUtility::RenderTooltip("The ID that determines what message is displayed by a Portrait Ghost when its heart is scanned by the GameBoy Horror.");

	// Comboboxes
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	LUIUtility::RenderNodeReferenceCombo<LItemAppearDOMNode>("Defeated Item Table", EDOMNodeType::ItemAppear, mapNode, mItemTableRef);
	LUIUtility::RenderTooltip("The Item Appear entry to use to spawn items when this character is defeated.");

	LUIUtility::RenderComboEnum<EConditionType>("Spawn Condition", mCondType);
	LUIUtility::RenderTooltip("The condition governing whether the character is allowed to spawn.");

	// This may be unused/useless?
	//LUIUtility::RenderComboEnum<EAppearType>("Spawn Type", mAppearType);
	//LUIUtility::RenderTooltip("The way in which the character will spawn. Using the second option WILL crash the game.");

	LUIUtility::RenderComboEnum<EAttackType>("Behavior Type", mAttackType);
	LUIUtility::RenderTooltip("How this character will behave.");

	// Bools
	LUIUtility::RenderCheckBox("Visible", &mIsVisible);
	LUIUtility::RenderTooltip("Whether this character should be visible.");

	LUIUtility::RenderCheckBox("Always Active", &mStay);
	LUIUtility::RenderTooltip("Keeps actor loaded regardless of current room");
}

void LCharacterDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{

	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "create_name", mCreateName);
	JmpIO->SetString(entry_index, "path_name", mPathName);
	JmpIO->SetString(entry_index, "CodeName", mCodeName);
	
	JmpIO->SetFloat(entry_index, "pos_x", mPosition.z);
	JmpIO->SetFloat(entry_index, "pos_y", mPosition.y);
	JmpIO->SetFloat(entry_index, "pos_z", mPosition.x);

	JmpIO->SetFloat(entry_index, "dir_x", mRotation.z);
	JmpIO->SetFloat(entry_index, "dir_y", -mRotation.y);
	JmpIO->SetFloat(entry_index, "dir_z", mRotation.x);

	JmpIO->SetUnsignedInt(entry_index, "room_no", mRoomNumber);

	JmpIO->SetUnsignedInt(entry_index, "appear_flag", mSpawnFlag);
	JmpIO->SetUnsignedInt(entry_index, "disappear_flag", mDespawnFlag);

	JmpIO->SetUnsignedInt(entry_index, "event_set_no", mEventSetNumber);
	JmpIO->SetUnsignedInt(entry_index, "item_table", mItemTableIndex);
	JmpIO->SetUnsignedInt(entry_index, "appear_point", mSpawnPointID);
	JmpIO->SetUnsignedInt(entry_index, "msg_no", mGBHScanID);

	JmpIO->SetUnsignedInt(entry_index, "cond_type", (uint32_t)mCondType);
	JmpIO->SetUnsignedInt(entry_index, "attack_type", (uint32_t)mAttackType);
	JmpIO->SetUnsignedInt(entry_index, "move_type", mMoveType);
	JmpIO->SetUnsignedInt(entry_index, "appear_type", (uint32_t)mAppearType);

	JmpIO->SetBoolean(entry_index, "invisible", mIsVisible);
	JmpIO->SetBoolean(entry_index, "stay", mStay);
}

void LCharacterDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mCreateName = JmpIO->GetString(entry_index, "create_name");
	mPathName = JmpIO->GetString(entry_index, "path_name");
	mCodeName = JmpIO->GetString(entry_index, "CodeName");

	mPosition.z = JmpIO->GetFloat(entry_index, "pos_x");
	mPosition.y = JmpIO->GetFloat(entry_index, "pos_y");
	mPosition.x = JmpIO->GetFloat(entry_index, "pos_z");

	mRotation.z = JmpIO->GetFloat(entry_index, "dir_x");
	mRotation.y = -JmpIO->GetFloat(entry_index, "dir_y");
	mRotation.x = JmpIO->GetFloat(entry_index, "dir_z");

	mRoomNumber = JmpIO->GetSignedInt(entry_index, "room_no");

	mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
	mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");

	mEventSetNumber = JmpIO->GetSignedInt(entry_index, "event_set_no");
	mItemTableIndex = JmpIO->GetSignedInt(entry_index, "item_table");
	mSpawnPointID = JmpIO->GetSignedInt(entry_index, "appear_point");
	mGBHScanID = JmpIO->GetSignedInt(entry_index, "msg_no");

	mCondType = (EConditionType)JmpIO->GetUnsignedInt(entry_index, "cond_type");
	mAttackType = (EAttackType)JmpIO->GetUnsignedInt(entry_index, "attack_type");
	mMoveType = JmpIO->GetUnsignedInt(entry_index, "move_type");
	mAppearType = (EAppearType)JmpIO->GetUnsignedInt(entry_index, "appear_type");

	mIsVisible = JmpIO->GetBoolean(entry_index, "invisible");
	mStay = JmpIO->GetBoolean(entry_index, "stay");
}

void LCharacterDOMNode::PostProcess()
{
	// On the off chance that the parent is invalid, don't try to do anything.
	if (Parent.expired())
		return;

	// Grab a temporary shared_ptr for the parent.
	auto parentShared = Parent.lock();

	// Grab item info data from the map
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		auto itemAppearNodes = mapNodeLocked->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);
		
		if (mItemTableIndex < itemAppearNodes.size())
			mItemTableRef = itemAppearNodes[mItemTableIndex];
	}
}

void LCharacterDOMNode::PreProcess()
{
	// On the off chance that the parent is invalid, don't try to do anything.
	if (Parent.expired())
		return;

	// Grab a temporary shared_ptr for the parent.
	auto parentShared = Parent.lock();

	if (mItemTableRef.expired())
	{
		mItemTableIndex = 0;
		return;
	}

	// Grab item info data from the map
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		auto itemAppearNodes = mapNodeLocked->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);

		auto lockedItemRef = mItemTableRef.lock();
		std::ptrdiff_t index = LGenUtility::VectorIndexOf(itemAppearNodes, lockedItemRef);

		if (index == -1)
			mItemTableIndex = 0;
		else
			mItemTableIndex = static_cast<int32_t>(index);
	}
}
