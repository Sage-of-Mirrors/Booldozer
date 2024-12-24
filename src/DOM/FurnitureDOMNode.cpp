#include "DOM/FurnitureDOMNode.hpp"
#include "DOM/ItemAppearDOMNode.hpp"
#include "UIUtil.hpp"

LFurnitureDOMNode::LFurnitureDOMNode(std::string name) : LEntityDOMNode(name),
	mModelName("(null)"), mAccessName("(null)"),
	mVerticalItemSpawnOffset(0.0f), mItemTableIndex(0), mGenerateNumber(0), mBooHideChance(0),
	mShakeIntensity(0), mVecArgs(glm::vec3(0.0f, 0.0f, 0.0f)), mSpawnFlag(0), mDespawnFlag(0),
	mHitboxExtents(glm::ivec3(0, 0, 0)), mGBHScanID(0), mBehaviorType(EMoveType::Heavy_No_Open), mSoundID(EFurnitureSound::Door_Opening), mSheetBehavior(ESheetBehavior::None),
	mMoneyType(EMoneyType::None), mSheetTexture(ESheetTexture::Tablecloth_Gold), mShouldCutaway(false), mCanSheetBeVaccuumed(false), mBooAppear(false)
{
	mType = EDOMNodeType::Furniture;
}

//todo
void LFurnitureDOMNode::CopyTo(LFurnitureDOMNode* other){
	other->mModelName = mModelName;
	other->mAccessName = mAccessName;
	other->mVerticalItemSpawnOffset = mVerticalItemSpawnOffset;
	other->mItemTableIndex = mItemTableIndex;
	other->mGenerateNumber = mGenerateNumber;
	other->mBooHideChance = mBooHideChance;
	other->mShakeIntensity = mShakeIntensity;
	other->mVecArgs = mVecArgs;
	other->mSpawnFlag = mSpawnFlag;
	other->mDespawnFlag = mDespawnFlag;
	other->mHitboxExtents = mHitboxExtents;
	other->mGBHScanID = mGBHScanID;
	other->mBehaviorType = mBehaviorType;
	other->mSoundID = mSoundID;
	other->mSheetBehavior = mSheetBehavior;
	other->mMoneyType = mMoneyType;
	other->mSheetTexture = mSheetTexture;
	other->mShouldCutaway = mShouldCutaway;
	other->mCanSheetBeVaccuumed = mCanSheetBeVaccuumed;
	other->mBooAppear = mBooAppear;
	other->mItemTableRef = mItemTableRef;
	other->mRoomNumber = mRoomNumber;
	other->mPosition = mPosition;
	other->mRotation = mRotation;
	other->mScale = mScale;
}

void LFurnitureDOMNode::RenderDetailsUI(float dt)
{
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

	std::weak_ptr<LRoomDOMNode> roomNode = GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room);
	if(ImGui::BeginCombo("Model", mModelName.c_str())){
		for(auto furniture : roomNode.lock()->GetAvailableFurniture()){
			if(ImGui::Selectable(furniture.c_str())){
				mModelName = furniture;
			}
		}
		ImGui::EndCombo();
	}
	//LUIUtility::RenderTextInput("Model", &mModelName);

	LUIUtility::RenderTextInput("Name", &mName);
	LUIUtility::RenderTooltip("The name of this furniture object. The game doesn't do anything with this, so feel free to use it for notes.");

	LUIUtility::RenderTextInput("Tag", &mAccessName);
	LUIUtility::RenderTooltip("A unique identifier that other objects will use to reference this piece of furniture. Often used by ghosts to be hidden inside furniture.");

	ImGui::InputFloat("Item Spawn Vertical Offset", &mVerticalItemSpawnOffset);
	LUIUtility::RenderTooltip("The vertical offset of the item spawned from this furniture object, relative to the furniture's position.");

	LUIUtility::RenderComboEnum<EMoneyType>("Money Type", mMoneyType);
	LUIUtility::RenderTooltip("If this piece of furniture is supposed to spawn money, this is what kind of currency it spawns.");

	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	LUIUtility::RenderNodeReferenceCombo<LItemAppearDOMNode>("Opened Item Table", EDOMNodeType::ItemAppear, mapNode, mItemTableRef);
	LUIUtility::RenderTooltip("The Item Appear entry to use to spawn items when this piece of furniture is opened.");

	ImGui::InputInt("Amount of Money Spawned", &mGenerateNumber);
	LUIUtility::RenderTooltip("If this piece of furniture is supposed to spawn money, this value is how much if it is spawned.");

	ImGui::InputInt("Boo Preference", &mBooHideChance);
	LUIUtility::RenderTooltip("How likely it is that a Boo will hide inside this piece of furniture. The higher this value, the more likely it is.");

	ImGui::InputInt("Shake Intensity", &mShakeIntensity);
	LUIUtility::RenderTooltip("How intensely this piece of furniture will shake when Luigi interacts with it.");

	ImGui::InputFloat("arg0", &mVecArgs.x);
	ImGui::InputFloat("arg1", &mVecArgs.y);
	ImGui::InputFloat("arg2", &mVecArgs.z);

	ImGui::InputInt("Spawn Flag", &mSpawnFlag);
	LUIUtility::RenderTooltip("The flag that must be set before this piece of furniture will begin spawning.");

	ImGui::InputInt("Despawn Flag", &mDespawnFlag);
	LUIUtility::RenderTooltip("If this flag is set, this piece of furniture will no longer spawn.");

	if (ImGui::TreeNode("Interaction Hitbox"))
	{
		ImGui::Indent();
		ImGui::InputInt("X", &mHitboxExtents.x);
		ImGui::InputInt("Y", &mHitboxExtents.y);
		ImGui::InputInt("Z", &mHitboxExtents.z);
		ImGui::Unindent();

		ImGui::TreePop();
	}
	LUIUtility::RenderTooltip("The side lengths of the hitbox that allows Luigi to interact with this furniture object. The hitbox is centered at the furniture's position.");

	ImGui::InputInt("GBH Scan ID", &mGBHScanID);
	LUIUtility::RenderTooltip("The ID that determines what Luigi will say about this piece of furniture when scanned with the GameBoy Horror.");

	// Comboboxes
	LUIUtility::RenderComboEnum<EMoveType>("Move Type", mBehaviorType);
	LUIUtility::RenderTooltip("How Luigi can interact with this piece of furniture.");

	LUIUtility::RenderComboEnum<EFurnitureSound>("Sound", mSoundID);
	LUIUtility::RenderTooltip("The sound played when Luigi interacts with this piece of furniture.");

	LUIUtility::RenderComboEnum<ESheetBehavior>("Sheet Behavior", mSheetBehavior);
	LUIUtility::RenderTooltip("Whether a sheet is placed on this piece of furniture, and if so, how it behaves.");
	if (mSheetBehavior != ESheetBehavior::None)
	{
		ImGui::Indent();
		LUIUtility::RenderComboEnum<ESheetTexture>("Sheet Texture", mSheetTexture);
		LUIUtility::RenderTooltip("The texture that the sheet placed on this piece of furniture will use.");

		LUIUtility::RenderCheckBox("Vaccuumable", &mCanSheetBeVaccuumed);
		LUIUtility::RenderTooltip("Whether Luigi can vaccuum up the sheet. If unchecked, the sheet will eventually snap back and damage Luigi when vaccuumed.");
		ImGui::Unindent();
	}

	// Bools
	LUIUtility::RenderCheckBox("Cutaway", &mShouldCutaway);
	LUIUtility::RenderTooltip("Whether this piece of furniture should disappear when Luigi walks behind it.");
}

void LFurnitureDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "dmd_name", mModelName);
	JmpIO->SetString(entry_index, "access_name", mAccessName);

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

	JmpIO->SetFloat(entry_index, "item_offset_y", mVerticalItemSpawnOffset);

	JmpIO->SetUnsignedInt(entry_index, "item_table", mItemTableIndex);
	JmpIO->SetUnsignedInt(entry_index, "generate_num", mGenerateNumber);
	JmpIO->SetUnsignedInt(entry_index, "telesa_hide", mBooHideChance);
	JmpIO->SetUnsignedInt(entry_index, "move_level", mShakeIntensity);

	JmpIO->SetFloat(entry_index, "arg0", mVecArgs.x);
	JmpIO->SetFloat(entry_index, "arg1", mVecArgs.y);
	JmpIO->SetFloat(entry_index, "arg2", mVecArgs.z);

	JmpIO->SetUnsignedInt(entry_index, "appear_flag", mSpawnFlag);
	JmpIO->SetUnsignedInt(entry_index, "disappear_flag", mDespawnFlag);

	JmpIO->SetUnsignedInt(entry_index, "furniture_x", mHitboxExtents.z);
	JmpIO->SetUnsignedInt(entry_index, "furniture_y", mHitboxExtents.y);
	JmpIO->SetUnsignedInt(entry_index, "furniture_z", mHitboxExtents.x);

	JmpIO->SetUnsignedInt(entry_index, "counter", mGBHScanID);

	JmpIO->SetUnsignedInt(entry_index, "move", (uint32_t)mBehaviorType);
	JmpIO->SetUnsignedInt(entry_index, "furniture_sound", (uint32_t)mSoundID);
	JmpIO->SetUnsignedInt(entry_index, "sheet", (uint32_t)mSheetBehavior);
	JmpIO->SetUnsignedInt(entry_index, "generate", (uint32_t)mMoneyType);
	JmpIO->SetUnsignedInt(entry_index, "sheet_texture", (uint32_t)mSheetTexture);

	JmpIO->SetBoolean(entry_index, "not_transparent", mShouldCutaway);
	JmpIO->SetBoolean(entry_index, "sheet_gum", !mCanSheetBeVaccuumed);

	JmpIO->SetBoolean(entry_index, "telesa_appear", mBooAppear);
}

void LFurnitureDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mModelName = JmpIO->GetString(entry_index, "dmd_name");
	mAccessName = JmpIO->GetString(entry_index, "access_name");

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

	mHitboxExtents.z = JmpIO->GetSignedInt(entry_index, "furniture_x");
	mHitboxExtents.y = JmpIO->GetSignedInt(entry_index, "furniture_y");
	mHitboxExtents.x = JmpIO->GetSignedInt(entry_index, "furniture_z");

	mGBHScanID = JmpIO->GetSignedInt(entry_index, "counter");

	mBehaviorType = (EMoveType)JmpIO->GetUnsignedInt(entry_index, "move");
	mSoundID = (EFurnitureSound)JmpIO->GetUnsignedInt(entry_index, "furniture_sound");
	mSheetBehavior = (ESheetBehavior)JmpIO->GetUnsignedInt(entry_index, "sheet");
	mMoneyType = (EMoneyType)JmpIO->GetUnsignedInt(entry_index, "generate");
	mSheetTexture = (ESheetTexture)JmpIO->GetUnsignedInt(entry_index, "sheet_texture");

	mShouldCutaway = JmpIO->GetBoolean(entry_index, "not_transparent");
	mCanSheetBeVaccuumed = !JmpIO->GetBoolean(entry_index, "sheet_gum");

	mBooAppear = JmpIO->GetBoolean(entry_index, "telesa_appear");
}

void LFurnitureDOMNode::PostProcess()
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

void LFurnitureDOMNode::PreProcess()
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
