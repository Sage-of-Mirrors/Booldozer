#include "DOM/DoorDOMNode.hpp"
#include "io/StaticMapDataIO.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "imgui.h"

LDoorDOMNode::LDoorDOMNode(std::string name) : Super(name),
	mOrientation(EDoorOrientation::Front_Facing), mDoorType(EDoorType::Door), mJmpId(0),
	mModel(EDoorModel::Square_Mansion_Door), mDoorEntryNumber(0), mNextEscape(0), mCurrentEscape(0)
{
	mType = EDOMNodeType::Door;
}

std::string LDoorDOMNode::GetName()
{
	std::string name = "";

	switch (mOrientation)
	{
	case EDoorOrientation::Front_Facing:
		name = LGenUtility::Format(name, u8"↕ ");
		break;
	case EDoorOrientation::Side_Facing:
		name = LGenUtility::Format(name, u8"↔ ");
		break;
	case EDoorOrientation::No_Fade:
		name = LGenUtility::Format(name, "No Fade ");
		break;
	default:
		break;
	}

	switch (mDoorType)
	{
	case EDoorType::Viewport:
		name = LGenUtility::Format(name, "(VP) ");
		break;
	case EDoorType::Window:
		name = LGenUtility::Format(name, "(W) ");
		break;
	}

	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		name = LGenUtility::Format(name, mapNodeLocked->GetRoomByID(mNextEscape)->GetName());
		name = LGenUtility::Format(name, "/", mapNodeLocked->GetRoomByID(mCurrentEscape)->GetName());
	}

	return name;
}

void LDoorDOMNode::RenderDetailsUI(float dt)
{
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

	LUIUtility::RenderComboEnum<EDoorOrientation>("Orientation", mOrientation);
	LUIUtility::RenderComboEnum<EDoorType>("Type", mDoorType);
	ImGui::InputInt("JMP ID", &mJmpId);
	LUIUtility::RenderComboEnum<EDoorModel>("Model", mModel);
	ImGui::InputInt("Entry Number", &mDoorEntryNumber);

	ImGui::InputInt("Next Escape", &mNextEscape);
	ImGui::InputInt("Current Escape", &mCurrentEscape);
}

bool LDoorDOMNode::Load(const LStaticDoorData& source)
{
	mOrientation = (EDoorOrientation)source.mOrientation;
	mDoorType = (EDoorType)source.mType;
	mJmpId = source.mJmpID;
	mModel = (EDoorModel)source.mModel;
	mDoorEntryNumber = source.mEntryIndex;

	mPosition = glm::vec3((float)source.mPosition.z, (float)source.mPosition.y, (float)source.mPosition.x);
	mViewportSize = glm::vec3((float)source.mViewportSize.z, (float)source.mViewportSize.y, (float)source.mViewportSize.x);

	mNextEscape = source.mNextEscape;
	mCurrentEscape = source.mCurrentEscape;

	return true;
}

bool LDoorDOMNode::Save(LStaticDoorData& dest)
{
	dest.mOrientation = (uint8_t)mOrientation;
	dest.mType = (uint8_t)mDoorType;
	dest.mJmpID = mJmpId;
	dest.mModel = (uint8_t)mModel;
	dest.mEntryIndex = mDoorEntryNumber;

	dest.mPosition.x = (uint32_t)mPosition.z;
	dest.mPosition.y = (uint32_t)mPosition.y;
	dest.mPosition.z = (uint32_t)mPosition.x;

	dest.mViewportSize.x = (uint16_t)mViewportSize.z;
	dest.mViewportSize.y = (uint16_t)mViewportSize.y;
	dest.mViewportSize.z = (uint16_t)mViewportSize.z;

	dest.mNextEscape = mNextEscape;
	dest.mCurrentEscape = mCurrentEscape;

	return true;
}
