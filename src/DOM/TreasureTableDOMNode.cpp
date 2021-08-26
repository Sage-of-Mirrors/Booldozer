#include "DOM/TreasureTableDOMNode.hpp"
#include "DOM/ItemInfoDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"

LTreasureTableDOMNode::LTreasureTableDOMNode(std::string name) : Super(name),
    mCoins(0), mBills(0), mGoldBars(0), mSmallPearls(0), mMediumPearls(0), mLargePearls(0),
    mSapphires(0), mEmeralds(0), mRubies(0), mDiamonds(0), mRedDiamonds(0), mGoldDiamonds(0),
    mSize(EChestSize::Small), mEffect(false), mCamera(false), mOther("nothing"), mTargetRoomNumber(-1),
    mOtherItemRef(std::weak_ptr<LItemInfoDOMNode>()), mRoomNodeRef(std::weak_ptr<LRoomDOMNode>())
{
    mType = EDOMNodeType::TreasureTable;
    mRoomNumber = -1;
}

void LTreasureTableDOMNode::RenderDetailsUI(float dt)
{
    // Integers
    ImGui::InputInt("Coins", &mCoins);
    LUIUtility::RenderTooltip("The number of coins that are inside this chest.");

    ImGui::InputInt("Bills", &mBills);
    LUIUtility::RenderTooltip("The number of bills that are inside this chest.");

    ImGui::InputInt("Gold Bars", &mGoldBars);
    LUIUtility::RenderTooltip("The number of gold bars that are inside this chest.");

    ImGui::InputInt("Small Pearls", &mSmallPearls);
    LUIUtility::RenderTooltip("The number of small pearls that are inside this chest.");

    ImGui::InputInt("Medium Pearls", &mMediumPearls);
    LUIUtility::RenderTooltip("The number of meidum pearls that are inside this chest.");

    ImGui::InputInt("Large Pearls", &mLargePearls);
    LUIUtility::RenderTooltip("The number of large pearls that are inside this chest.");

    ImGui::InputInt("Sapphires", &mSapphires);
    LUIUtility::RenderTooltip("The number of sapphires that are inside this chest.");

    ImGui::InputInt("Emeralds", &mEmeralds);
    LUIUtility::RenderTooltip("The number of emeralds that are inside this chest.");

    ImGui::InputInt("Rubies", &mRubies);
    LUIUtility::RenderTooltip("The number of rubies that are inside this chest.");

    ImGui::InputInt("Diamonds", &mDiamonds);
    LUIUtility::RenderTooltip("The number of diamonds that are inside this chest.");

    ImGui::InputInt("Red Diamonds", &mRedDiamonds);
    LUIUtility::RenderTooltip("The number of red (fake) diamonds that are inside this chest.");

    ImGui::InputInt("Gold Diamonds", &mGoldDiamonds);
    LUIUtility::RenderTooltip("The number of gold diamonds that are inside this chest.");

    // Comboboxes
    LUIUtility::RenderComboEnum<EChestSize>("Size", mSize);
    LUIUtility::RenderTooltip("How large the chest is.");

    LUIUtility::RenderNodeReferenceCombo<LItemInfoDOMNode>("Other Item", EDOMNodeType::ItemInfo, Parent, mOtherItemRef);
    LUIUtility::RenderTooltip("The item from the master item list to spawn from the chest.");

    // Showing the room combo requires us to get the map node. It shouldn't ever be expired, but safety first I guess.
    auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
    if (!mapNode.expired())
    {
        LUIUtility::RenderNodeReferenceCombo<LRoomDOMNode>("Room", EDOMNodeType::Room, mapNode, mRoomNodeRef);
        LUIUtility::RenderTooltip("The room that this chest data is for.");
    }

    // Bools
    LUIUtility::RenderCheckBox("Chest has Sparkling Particles?", &mEffect);
    LUIUtility::RenderTooltip("Whether this chest should have sparkles floating around it.");

    LUIUtility::RenderCheckBox("Focus Camera on Chest?", &mCamera);
    LUIUtility::RenderTooltip("Whether the camera should focus on this chest when it spawns, or when Luigi leaves and re-enters the room without opening it.");
}

void LTreasureTableDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    JmpIO->SetString(entry_index, "other", mOther);
    JmpIO->SetSignedInt(entry_index, "room", mTargetRoomNumber);

    JmpIO->SetSignedInt(entry_index, "size", (int32_t)mSize);

    JmpIO->SetSignedInt(entry_index, "coin", mCoins);
    JmpIO->SetSignedInt(entry_index, "bill", mBills);
    JmpIO->SetSignedInt(entry_index, "gold", mGoldBars);
    JmpIO->SetSignedInt(entry_index, "SPearl", mSmallPearls);
    JmpIO->SetSignedInt(entry_index, "MPearl", mMediumPearls);
    JmpIO->SetSignedInt(entry_index, "LPearl", mLargePearls);
    JmpIO->SetSignedInt(entry_index, "sapphire", mSapphires);
    JmpIO->SetSignedInt(entry_index, "emerald", mEmeralds);
    JmpIO->SetSignedInt(entry_index, "ruby", mRubies);
    JmpIO->SetSignedInt(entry_index, "diamond", mDiamonds);
    JmpIO->SetSignedInt(entry_index, "cdiamond", mRedDiamonds);
    JmpIO->SetSignedInt(entry_index, "rdiamond", mGoldDiamonds);

    JmpIO->SetBoolean(entry_index, "effect", mEffect);
    JmpIO->SetBoolean(entry_index, "camera", mCamera);
}

void LTreasureTableDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    mOther = JmpIO->GetString(entry_index, "other");
    mTargetRoomNumber = JmpIO->GetSignedInt(entry_index, "room");

    mSize = (EChestSize)JmpIO->GetSignedInt(entry_index, "size");

    mCoins = JmpIO->GetSignedInt(entry_index, "coin");
    mBills = JmpIO->GetSignedInt(entry_index, "bill");
    mGoldBars = JmpIO->GetSignedInt(entry_index, "gold");
    mSmallPearls = JmpIO->GetSignedInt(entry_index, "SPearl");
    mMediumPearls = JmpIO->GetSignedInt(entry_index, "MPearl");
    mLargePearls = JmpIO->GetSignedInt(entry_index, "LPearl");
    mSapphires = JmpIO->GetSignedInt(entry_index, "sapphire");
    mEmeralds = JmpIO->GetSignedInt(entry_index, "emerald");
    mRubies = JmpIO->GetSignedInt(entry_index, "ruby");
    mDiamonds = JmpIO->GetSignedInt(entry_index, "diamond");
    mRedDiamonds = JmpIO->GetSignedInt(entry_index, "cdiamond");
    mGoldDiamonds = JmpIO->GetSignedInt(entry_index, "rdiamond");

    mEffect = JmpIO->GetBoolean(entry_index, "effect");
    mCamera = JmpIO->GetBoolean(entry_index, "camera");
}

void LTreasureTableDOMNode::PostProcess()
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
        // Set other item reference
        auto itemInfos = mapNodeLocked->GetChildrenOfType<LItemInfoDOMNode>(EDOMNodeType::ItemInfo);
        for (auto info : itemInfos)
        {
            if (info->GetName() == mOther)
            {
                mOtherItemRef = info;
                break;
            }
        }

        // Set room reference
        auto rooms = mapNodeLocked->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
        if (mTargetRoomNumber >= rooms.size())
        {
            std::cout << LGenUtility::Format("Invalid target room number \"", mTargetRoomNumber, "\" on treasure table \"", mName, "\"!");
            return;
        }
        else
            mRoomNodeRef = rooms[mTargetRoomNumber];
    }
}

void LTreasureTableDOMNode::PreProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();

    if (auto info = mOtherItemRef.lock())
        mOther = info->GetName();
    else
        mOther = "nothing";

    if (auto room = mRoomNodeRef.lock())
        mTargetRoomNumber = room->GetRoomNumber();
    else
        mTargetRoomNumber = -1;
}
