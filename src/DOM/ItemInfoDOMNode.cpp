#include "DOM/ItemInfoDOMNode.hpp"
#include "UIUtil.hpp"

LItemInfoDOMNode::LItemInfoDOMNode(std::string name) : Super(name),
	mCharacterName("(null)"), mOpenDoorNumber(0), mHPAmount(0), mAllowMovement(false)
{
	mType = EDOMNodeType::ItemInfo;
	mRoomNumber = -1;
}

void LItemInfoDOMNode::RenderDetailsUI(float dt)
{
	// Strings
	LUIUtility::RenderTextInput("Item Name", &mName);
	LUIUtility::RenderTooltip("");

	LUIUtility::RenderTextInput("Actor Name", &mCharacterName);
	LUIUtility::RenderTooltip("");

	// Integers
	ImGui::InputInt("Open Door Number", &mOpenDoorNumber);
	LUIUtility::RenderTooltip("");

	//ImGui::InputInt("HP Amount", &mHPAmount);
	//LUIUtility::RenderTooltip("");

	// Bools
	LUIUtility::RenderCheckBox("Allow Movement?", &mAllowMovement);
	LUIUtility::RenderTooltip("");
}

void LItemInfoDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "character_name", mCharacterName);

	JmpIO->SetSignedInt(entry_index, "OpenDoorNo", mOpenDoorNumber);
	//JmpIO->SetSignedInt(entry_index, "str_hp", mHPAmount);

	JmpIO->SetBoolean(entry_index, "IsEscape", mAllowMovement);
}

void LItemInfoDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mCharacterName = JmpIO->GetString(entry_index, "character_name");

	mOpenDoorNumber = JmpIO->GetSignedInt(entry_index, "OpenDoorNo");
	//mHPAmount = JmpIO->GetSignedInt(entry_index, "str_hp");

	mAllowMovement = JmpIO->GetBoolean(entry_index, "IsEscape");
}

void LItemInfoDOMNode::PostProcess()
{

}

void LItemInfoDOMNode::PreProcess()
{

}
