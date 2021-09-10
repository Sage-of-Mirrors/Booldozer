#include "DOM/EnemyDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "DOM/FurnitureDOMNode.hpp"
#include "DOM/ItemAppearDOMNode.hpp"
#include <sstream>

std::map<std::string, std::string> EnemyNames = {
    { "yapoo1", "Gold Ghost" },
    { "yapoo2", "Temper Terror" },

    { "mapoo1", "Purple Puncher" },
    { "mapoo2", "Flash" },

    { "mopoo1", "Blue Twirler" },
    { "mopoo2", "Blue Blaze" },

    { "banaoba", "Garbage Can Ghost" },

    { "topoo1", "Red Grabbing Ghost" },
    { "topoo2", "Mirror Ghost" },
    { "topoo3", "Cinema Ghost" },
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
    { "iyapoo13", "Speedy Spirit 13" },
    { "iyapoo14", "Speedy Spirit 14" },
    { "iyapoo15", "Speedy Spirit 15" },
    { "iyapoo16", "Speedy Spirit 16" },
    { "iyapoo17", "Speedy Spirit 17" },
    { "iyapoo18", "Speedy Spirit 18" },
    { "iyapoo19", "Speedy Spirit 19" },
    { "iyapoo20", "Speedy Spirit 20" },

    { "piero1", "Red Clown Doll" },
    { "piero2", "Blue Clown Doll" },

    { "heypo1", "Red Shy Guy Ghost" },
    { "heypo2", "Green Shy Guy Ghost" },
    { "heypo3", "White Shy Guy Ghost" },
    { "heypo4", "Brown Shy Guy Ghost" },
    { "heypo5", "Orange Shy Guy Ghost" },
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
    { "polter1", "Flying Object Manager" },

    { "skul", "Skeleton Ghost" },

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
    mFloatingHeight(0), mAppearChance(64), mSpawnFlag(0), mDespawnFlag(0), mEventSetNumber(0), mItemTableIndex(0),
    mCondType(EConditionType::Always_True), mMoveType(0), mSearchType(0), mAppearType(EAppearType::Normal), mPlaceType(EPlaceType::Always_Spawn_at_Initial_Position), mIsVisible(true), mStay(false),
    mFurnitureNodeRef(std::weak_ptr<LFurnitureDOMNode>()), mIsBlackoutEnemy(false)
{
    mType = EDOMNodeType::Enemy;
}

void LEnemyDOMNode::RenderDetailsUI(float dt)
{
    LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

    LUIUtility::RenderComboBox("Enemy Type", EnemyNames, mName);
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
    LUIUtility::RenderCheckBox("Visible?", &mIsVisible);
    LUIUtility::RenderTooltip("Whether this enemy should be visible.");

    LUIUtility::RenderCheckBox("Stay on path between rooms?", &mStay);
    LUIUtility::RenderTooltip("Whether this enemy should remain on a path when Luigi moves between rooms.");
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

    JmpIO->SetSignedInt(entry_index, "event_set_no", mEventSetNumber);
    JmpIO->SetSignedInt(entry_index, "item_table", mItemTableIndex);

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
            ptrdiff_t furnitureIndex = LGenUtility::VectorIndexOf(furnitureNodes, furnitureShared);
            if (furnitureIndex == -1)
            {
                std::cout << "Tried to set furniture access name to nonexistent furniture node!";
                return;
            }

            // Set the new access name in this enemy and in the associated furniture node.
            mAccessName = LGenUtility::Format(mRoomNumber, '_', furnitureIndex);
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
        ptrdiff_t index = LGenUtility::VectorIndexOf(itemAppearNodes, lockedItemRef);

        if (index == -1)
            mItemTableIndex = 0;
        else
            mItemTableIndex = index;
    }
}
