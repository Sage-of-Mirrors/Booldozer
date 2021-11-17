#include "DOM/DoorDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "io/StaticMapDataIO.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "imgui.h"

LDoorDOMNode::LDoorDOMNode(std::string name) : Super(name),
	mOrientation(EDoorOrientation::Front_Facing), mDoorType(EDoorType::Door), mJmpId(0),
	mModel(EDoorModel::Square_Mansion_Door), mDoorEntryNumber(0), mNextEscape(0), mCurrentEscape(0),
	mWestSouthRoom(), mEastNorthRoom()
{
	mType = EDOMNodeType::Door;
}

std::string LDoorDOMNode::GetName()
{
	std::string name = "";

	name = LGenUtility::Format(name, "[", mJmpId, "] ");

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
		if (auto wsRoomLocked = mWestSouthRoom.lock())
			name = LGenUtility::Format(name, wsRoomLocked->GetName());
		else
			name = LGenUtility::Format(name, "<Invalid Room>");
		if (auto enRoomLocked = mEastNorthRoom.lock())
			name = LGenUtility::Format(name, "/", enRoomLocked->GetName());
		else
			name = LGenUtility::Format(name, "/" "<Invalid Room>");
	}

	return name;
}

void LDoorDOMNode::RenderDetailsUI(float dt)
{
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

	LUIUtility::RenderComboEnum<EDoorOrientation>("Orientation", mOrientation);
	LUIUtility::RenderComboEnum<EDoorType>("Type", mDoorType);
	LUIUtility::RenderComboEnum<EDoorModel>("Model", mModel);

	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	LUIUtility::RenderNodeReferenceCombo<LRoomDOMNode>("West/South Room", EDOMNodeType::Room, mapNode, mWestSouthRoom);
	LUIUtility::RenderNodeReferenceCombo<LRoomDOMNode>("East/North Room", EDOMNodeType::Room, mapNode, mEastNorthRoom);
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

void LDoorDOMNode::PostProcess()
{
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		mWestSouthRoom = mapNodeLocked->GetRoomByID(mNextEscape);
		mEastNorthRoom = mapNodeLocked->GetRoomByID(mCurrentEscape);
	}
}

void LDoorDOMNode::PreProcess()
{
	if (auto wsRoomLocked = mWestSouthRoom.lock())
		mNextEscape = wsRoomLocked->GetRoomID();
	if (auto enRoomLocked = mEastNorthRoom.lock())
		mCurrentEscape = enRoomLocked->GetRoomID();
}

void LDoorDOMNode::AssignJmpIdAndIndex(std::vector<std::shared_ptr<LDoorDOMNode>> doors)
{
	mDoorEntryNumber = doors.size() + 1;
	mJmpId = -1;

	int32_t highestSeen = -1;

	for (int i = 0; i < doors.size(); i++)
	{
		bool found = false;

		for (auto d : doors)
		{
			if (d->GetJmpId() > highestSeen)
				highestSeen = d->GetJmpId();

			if (d->GetJmpId() == i)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			mJmpId = i;
			break;
		}
	}

	if (mJmpId == -1)
		mJmpId = highestSeen++;

	/*
	// Find the highest JMP ID
	for (auto d : doors)
	{
		int32_t id = d->GetJmpId();

		if (id > mJmpId)
			mJmpId = id;
	}

	// Increment the highest JMP ID. We now have the highest ID!
	mJmpId++;
	*/
}
