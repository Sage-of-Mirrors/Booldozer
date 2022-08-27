#include "DOM/ObserverDOMNode.hpp"
#include "UIUtil.hpp"

std::map<std::string, std::string> ObserverNames = {
	{ "observer", "Logic" },
	{ "kinopio", "Toad" },
	{ "soundobj", "Sound Effect Player" },
	{ "otobira", "Clockwork Room Elevators" },
	{ "iphone", "Telephone" }
};

LObserverDOMNode::LObserverDOMNode(std::string name) : LObserverDOMNode(name, false)
{

}

LObserverDOMNode::LObserverDOMNode(std::string name, bool isBlackoutObserver) : Super(name, isBlackoutObserver),
	mCodeName("(null)"), mCondStringArg0("(null)"), mStringArg0("(null)"),
	mCondArg0(0), mArg0(0), mArg1(0), mArg2(0), mArg3(0), mArg4(0), mArg5(0), mSpawnFlag(0), mDespawnFlag(0),
	mCondType(EConditionType::Always_True), mDoType(EDoType::Nothing), mIsVisible(false), mUnkBool1(false)
{
	mType = EDOMNodeType::Observer;
}

void LObserverDOMNode::RenderDetailsUI(float dt)
{
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

	ImGui::InputInt("Room Number", &mRoomNumber);

	LUIUtility::RenderComboBox("Observer Type", ObserverNames, mName);
	LUIUtility::RenderTooltip("What kind of observer this actor is.");

	// Strings
	LUIUtility::RenderTextInput("Script Name", &mCodeName);
	LUIUtility::RenderTooltip("The name that can be used to reference this observer in an event.");

	LUIUtility::RenderTextInput("Spawn Group Name", &mCondStringArg0);
	LUIUtility::RenderTooltip("The name of the Spawn Group that this observer is associated with. When all entities in this spawn group are dead, the observer's Action will be executed.");

	LUIUtility::RenderTextInput("Argument String", &mStringArg0);
	LUIUtility::RenderTooltip("A general-purpose argument whose meaning depends on the observer type.");

	// Integers
	ImGui::InputInt("Flag ID", &mCondArg0);
	LUIUtility::RenderTooltip("The ID of the flag for this observer to check, if its Condition is Flag Set or Flag Unset.");

	ImGui::InputInt("Argument 0", &mArg0);
	LUIUtility::RenderTooltip("");
	ImGui::InputInt("Argument 1", &mArg1);
	LUIUtility::RenderTooltip("");
	ImGui::InputInt("Argument 2", &mArg2);
	LUIUtility::RenderTooltip("");
	ImGui::InputInt("Argument 3", &mArg3);
	LUIUtility::RenderTooltip("");
	ImGui::InputInt("Argument 4", &mArg4);
	LUIUtility::RenderTooltip("");
	
	// Useless?
	//ImGui::InputInt("Argument 0", &mArg0);
	//LUIUtility::RenderTooltip("");

	ImGui::InputInt("Spawn Flag", &mSpawnFlag);
	LUIUtility::RenderTooltip("The flag that must be set before this piece of furniture will begin spawning.");

	ImGui::InputInt("Despawn Flag", &mDespawnFlag);
	LUIUtility::RenderTooltip("If this flag is set, this piece of furniture will no longer spawn.");

	// Comboboxes
	LUIUtility::RenderComboEnum<EConditionType>("Condition", mCondType);
	LUIUtility::RenderTooltip("The condition that this observer is waiting to be fulfilled. When the condition is met, the observer's Action will be executed.");

	LUIUtility::RenderComboEnum<EDoType>("Action", mDoType);
	LUIUtility::RenderTooltip("The action that the observer will execute once its Condition has been met.");

	// Bools
	LUIUtility::RenderCheckBox("Visible?", &mIsVisible);
	LUIUtility::RenderTooltip("");

	LUIUtility::RenderCheckBox("Unknown Boolean 1", &mUnkBool1);
	LUIUtility::RenderTooltip("???");
}

void LObserverDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "CodeName", mCodeName);
	JmpIO->SetString(entry_index, "cond_string_arg0", mCondStringArg0);
	JmpIO->SetString(entry_index, "string_arg0", mStringArg0);

	JmpIO->SetFloat(entry_index, "pos_x", mPosition.z);
	JmpIO->SetFloat(entry_index, "pos_y", mPosition.y);
	JmpIO->SetFloat(entry_index, "pos_z", mPosition.x);

	JmpIO->SetFloat(entry_index, "dir_x", mRotation.z);
	JmpIO->SetFloat(entry_index, "dir_y", mRotation.y);
	JmpIO->SetFloat(entry_index, "dir_z", mRotation.x);

	JmpIO->SetFloat(entry_index, "scale_x", mScale.z);
	JmpIO->SetFloat(entry_index, "scale_y", mScale.y);
	JmpIO->SetFloat(entry_index, "scale_z", mScale.x);

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

	JmpIO->SetUnsignedInt(entry_index, "cond_type", (uint32_t)mCondType);
	JmpIO->SetUnsignedInt(entry_index, "do_type", (uint32_t)mDoType);

	JmpIO->SetBoolean(entry_index, "invisible", mIsVisible);
}

void LObserverDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mCodeName = JmpIO->GetString(entry_index, "CodeName");
	mCondStringArg0 = JmpIO->GetString(entry_index, "cond_string_arg0");
	mStringArg0 = JmpIO->GetString(entry_index, "string_arg0");

	mPosition.z = JmpIO->GetFloat(entry_index, "pos_x");
	mPosition.y = JmpIO->GetFloat(entry_index, "pos_y");
	mPosition.x = JmpIO->GetFloat(entry_index, "pos_z");

	mRotation.z = glm::radians(JmpIO->GetFloat(entry_index, "dir_x"));
	mRotation.y = glm::radians(JmpIO->GetFloat(entry_index, "dir_y"));
	mRotation.x = glm::radians(JmpIO->GetFloat(entry_index, "dir_z"));

	mScale.z = JmpIO->GetFloat(entry_index, "scale_x");
	mScale.y = JmpIO->GetFloat(entry_index, "scale_y");
	mScale.x = JmpIO->GetFloat(entry_index, "scale_z");

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

	mCondType = (EConditionType)JmpIO->GetUnsignedInt(entry_index, "cond_type");
	mDoType = (EDoType)JmpIO->GetUnsignedInt(entry_index, "do_type");

	mIsVisible = JmpIO->GetBoolean(entry_index, "invisible");
}

void LObserverDOMNode::PostProcess()
{

}

void LObserverDOMNode::PreProcess()
{

}
