#include "DOM/EnemyDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "DOM/FurnitureDOMNode.hpp"
#include "DOM/ItemAppearDOMNode.hpp"
#include <sstream>
#include <json.hpp>

LEnemyDOMNode::LEnemyDOMNode(std::string name) : LEnemyDOMNode(name, false)
{

}

LEnemyDOMNode::LEnemyDOMNode(std::string name, bool isBlackoutEnemy) : Super(name, isBlackoutEnemy)
{
    mType = EDOMNodeType::Enemy;
}

std::string LEnemyDOMNode::GetName()
{
    auto enemyNames = LResUtility::GetNameMap("enemies");
    for (auto& section : enemyNames.items())
    {
        for (auto& entry : section.value().items())
        {
            if (entry.key() == mName)
            {
                return entry.value().get<std::string>();
            }
        }
    }
    return "(null)";
}

void LEnemyDOMNode::CopyTo(LEnemyDOMNode* other){
    other->mRoomNumber = mRoomNumber;
    other->mName = mName;
    other->mCreateName = mCreateName;
    other->mPathName = mPathName;
    other->mAccessName = mAccessName;
    other->mCodeName = mCodeName;
    other->mFloatingHeight = mFloatingHeight;
    other->mAppearChance = mAppearChance;
    other->mSpawnFlag = mSpawnFlag;
    other->mDespawnFlag = mDespawnFlag;
    other->mEventSetNumber = mEventSetNumber;
    other->mCondType = mCondType;
    other->mAppearType = mAppearType;
    other->mPlaceType = mPlaceType;
    other->mIsVisible = mIsVisible;
    other->mStay = mStay;
    other->mPosition = mPosition;
}

void LEnemyDOMNode::RenderDetailsUI(float dt)
{
    LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

    ImGui::InputInt("Room Number", &mRoomNumber);

    if(LUIUtility::RenderComboBox("Enemy Type", LResUtility::GetNameMap("enemies"), mName)){
        LEditorScene::GetEditorScene()->LoadActor(mName, false);
    }
    LUIUtility::RenderTooltip("What kind of enemy this actor is.");

    // Strings
    LUIUtility::RenderTextInput("Spawn Group", &mCreateName);
    LUIUtility::RenderTooltip("What Spawn Group this enemy is in. Set this to ---- to spawn when Luigi enters the room. This is also known as create_name.");

    LUIUtility::RenderTextInput("Path Name", &mPathName);
    LUIUtility::RenderTooltip("The name of a path file that this enemy will use. Set this to (null) for no path.");

    LUIUtility::RenderTextInput("Furniture Tag", &mAccessName);
    LUIUtility::RenderTooltip("The tag of a furniture object that the enemy will hide inside. Set this to (null) to disable this feature.");

    LUIUtility::RenderTextInput("Script Name", &mCodeName);
    LUIUtility::RenderTooltip("The name that can be used to reference this enemy in an event.");

    // Integers
    ImGui::InputInt("Floating Height", &mFloatingHeight);
    LUIUtility::RenderTooltip("How far above or below the enemy's Y position to actually spawn the enemy.");

    ImGui::InputInt("Spawn Chance", &mAppearChance);
    LUIUtility::RenderTooltip("The % chance that the enemy will spawn.");

    ImGui::InputInt("Spawn Flag", &mSpawnFlag);
    LUIUtility::RenderTooltip("The flag that must be set before this enemy will begin spawning.");
    ImGui::InputInt("Despawn Flag", &mDespawnFlag);
    LUIUtility::RenderTooltip("If this flag is set, this enemy will no longer spawn.");

    ImGui::InputInt("event_set_no", &mEventSetNumber);
    LUIUtility::RenderTooltip("Don't quite know what this does. It's called event_set_no by the game, but it seems useless?");

    // Comboboxes
    LUIUtility::RenderNodeReferenceCombo<LFurnitureDOMNode>("Furniture Access", EDOMNodeType::Furniture, Parent, mFurnitureNodeRef);
    LUIUtility::RenderTooltip("Allows the enemy to be hidden within a piece of furniture. That furniture must be interacted with to spawn the enemy.");

    auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
    LUIUtility::RenderNodeReferenceCombo<LItemAppearDOMNode>("Defeated Item Table", EDOMNodeType::ItemAppear, mapNode, mItemTableRef);
    LUIUtility::RenderTooltip("The Item Appear entry to use to spawn items when this enemy is defeated.");

    LUIUtility::RenderComboEnum<EConditionType>("Spawn Condition", mCondType);
    LUIUtility::RenderTooltip("The condition governing whether the enemy is allowed to spawn.");

    LUIUtility::RenderComboEnum<EAppearType>("Spawn Type", mAppearType);
    LUIUtility::RenderTooltip("The way in which the enemy will spawn. Using the second option WILL crash the game.");

    LUIUtility::RenderComboEnum<EPlaceType>("Placement Type", mPlaceType);
    LUIUtility::RenderTooltip("The location at which the enemy will spawn. If Spawn on Path is selected, you must specify a valid Path Name.");

    // Bools
    LUIUtility::RenderCheckBox("Visible", &mIsVisible);
    LUIUtility::RenderTooltip("Whether this enemy should be visible.");

    LUIUtility::RenderCheckBox("Always Active", &mStay);
    LUIUtility::RenderTooltip("Keeps actor loaded regardless of current room");
}

void LEnemyDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    JmpIO->SetString(entry_index, "name", mName);
    JmpIO->SetString(entry_index, "create_name", mCreateName);
    JmpIO->SetString(entry_index, "path_name", mPathName);
    JmpIO->SetString(entry_index, "access_name", mAccessName);
    JmpIO->SetString(entry_index, "CodeName", mCodeName);

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

    JmpIO->SetUnsignedInt(entry_index, "floating_height", mFloatingHeight);
    JmpIO->SetUnsignedInt(entry_index, "appear_percent", mAppearChance);

    JmpIO->SetUnsignedInt(entry_index, "appear_flag", mSpawnFlag);
    JmpIO->SetUnsignedInt(entry_index, "disappear_flag", mDespawnFlag);

    JmpIO->SetUnsignedInt(entry_index, "event_set_no", mEventSetNumber);
    JmpIO->SetUnsignedInt(entry_index, "item_table", mItemTableIndex);

    JmpIO->SetUnsignedInt(entry_index, "cond_type", (uint32_t)mCondType);
    JmpIO->SetUnsignedInt(entry_index, "move_type", mMoveType);
    JmpIO->SetUnsignedInt(entry_index, "search_type", mSearchType);
    JmpIO->SetUnsignedInt(entry_index, "appear_type", (uint32_t)mAppearType);
    JmpIO->SetUnsignedInt(entry_index, "place_type", (uint32_t)mPlaceType);

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

    mFloatingHeight = JmpIO->GetSignedInt(entry_index, "floating_height");
    mAppearChance = JmpIO->GetSignedInt(entry_index, "appear_percent");

    mSpawnFlag = JmpIO->GetSignedInt(entry_index, "appear_flag");
    mDespawnFlag = JmpIO->GetSignedInt(entry_index, "disappear_flag");

    mEventSetNumber = JmpIO->GetSignedInt(entry_index, "event_set_no");
    mItemTableIndex = JmpIO->GetSignedInt(entry_index, "item_table");

    mCondType = (EConditionType)JmpIO->GetUnsignedInt(entry_index, "cond_type");
    mMoveType = JmpIO->GetUnsignedInt(entry_index, "move_type");
    mSearchType = JmpIO->GetUnsignedInt(entry_index, "search_type");
    mAppearType = (EAppearType)JmpIO->GetUnsignedInt(entry_index, "appear_type");
    mPlaceType = (EPlaceType)JmpIO->GetUnsignedInt(entry_index, "place_type");

    mIsVisible = JmpIO->GetBoolean(entry_index, "invisible");
    mStay = JmpIO->GetBoolean(entry_index, "stay");
}

void LEnemyDOMNode::PostProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();

    if (mAccessName != "(null)")
    {
        auto furnitureNodes = parentShared->GetChildrenOfType<LFurnitureDOMNode>(EDOMNodeType::Furniture);

        for (auto furnitureNode : furnitureNodes)
        {
            if (furnitureNode->GetAccessName() == mAccessName)
            {
                mFurnitureNodeRef = furnitureNode;
                break;
            }
        }
    }

    // Grab item info data from the map
    auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
    if (auto mapNodeLocked = mapNode.lock())
    {
        auto itemAppearNodes = mapNodeLocked->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);

        if (mItemTableIndex < itemAppearNodes.size())
            mItemTableRef = itemAppearNodes[mItemTableIndex];
    }
}

void LEnemyDOMNode::PreProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();

    // If the furniture reference is valid, set the access name.
    if (auto furnitureShared = mFurnitureNodeRef.lock())
    {
        // We'll allow the user to specify a furniture node without a valid access name;
        // we'll create one by combining the room number with the index of the furniture node.
        if (furnitureShared->GetAccessName() == "(null)")
        {
            auto furnitureNodes = parentShared->GetChildrenOfType<LFurnitureDOMNode>(EDOMNodeType::Furniture);

            // Grab the index of the furniture node in the list of furniture. Report an error if it doesn't exist, because that shouldn't happen.
            std::ptrdiff_t furnitureIndex = LGenUtility::VectorIndexOf(furnitureNodes, furnitureShared);
            if (furnitureIndex == -1)
            {
                LGenUtility::Log << "[EnemyDOMNode]: Tried to set furniture access name to nonexistent furniture node!";
                return;
            }

            // Set the new access name in this enemy and in the associated furniture node.
            mAccessName = std::format("{0}_{1}", mRoomNumber, furnitureIndex);
            furnitureShared->SetAccessName(mAccessName);
        }
        // Furniture already has a valid access name, so we'll just grab it.
        else
            mAccessName = furnitureShared->GetAccessName();
    }
    // Otherwise, if the furniture reference is invalid, reset the enemy's access name to null.
    else
        mAccessName = "(null)";

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
