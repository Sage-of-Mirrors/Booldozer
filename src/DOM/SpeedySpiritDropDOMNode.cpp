#include "DOM/SpeedySpiritDropDOMNode.hpp"
#include "UIUtil.hpp"

LSpeedySpiritDropDOMNode::LSpeedySpiritDropDOMNode(std::string name) : Super(name),
    mCoins(0), mBills(0), mGoldBars(0), mSapphires(0), mEmeralds(0), mRubies(0)
{
    mType = EDOMNodeType::SpeedySpiritDrop;
    mRoomNumber = -1;
}

void LSpeedySpiritDropDOMNode::RenderDetailsUI(float dt)
{
    // Strings
    LUIUtility::RenderTextInput("Name", &mName);
    LUIUtility::RenderTooltip("Which ghost this drop data is for.");

    // Integers
    ImGui::InputInt("Coins", &mCoins);
    LUIUtility::RenderTooltip("The number of coins that this ghost drops.");

    ImGui::InputInt("Bills", &mBills);
    LUIUtility::RenderTooltip("The number of bills that this ghost drops.");

    ImGui::InputInt("Gold Bars", &mGoldBars);
    LUIUtility::RenderTooltip("The number of gold bars that this ghost drops.");

    ImGui::InputInt("Sapphires", &mSapphires);
    LUIUtility::RenderTooltip("The number of sapphires that this ghost drops.");

    ImGui::InputInt("Emeralds", &mEmeralds);
    LUIUtility::RenderTooltip("The number of emeralds that this ghost drops.");

    ImGui::InputInt("Rubies", &mRubies);
    LUIUtility::RenderTooltip("The number of rubies that this ghost drops.");
}

void LSpeedySpiritDropDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    JmpIO->SetString(entry_index, "name", mName);

    JmpIO->SetUnsignedInt(entry_index, "coin", mCoins);
    JmpIO->SetUnsignedInt(entry_index, "bill", mBills);
    JmpIO->SetUnsignedInt(entry_index, "gold", mGoldBars);
    JmpIO->SetUnsignedInt(entry_index, "sapphire", mSapphires);
    JmpIO->SetUnsignedInt(entry_index, "emerald", mEmeralds);
    JmpIO->SetUnsignedInt(entry_index, "ruby", mRubies);
}

void LSpeedySpiritDropDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    mName = JmpIO->GetString(entry_index, "name");

    mCoins = JmpIO->GetSignedInt(entry_index, "coin");
    mBills = JmpIO->GetSignedInt(entry_index, "bill");
    mGoldBars = JmpIO->GetSignedInt(entry_index, "gold");
    mSapphires = JmpIO->GetSignedInt(entry_index, "sapphire");
    mEmeralds = JmpIO->GetSignedInt(entry_index, "emerald");
    mRubies = JmpIO->GetSignedInt(entry_index, "ruby");
}

void LSpeedySpiritDropDOMNode::PostProcess()
{

}

void LSpeedySpiritDropDOMNode::PreProcess()
{

}