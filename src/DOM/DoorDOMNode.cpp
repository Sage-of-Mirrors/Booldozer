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
	mScale = glm::vec3((float)source.mViewportSize.z, (float)source.mViewportSize.y, (float)source.mViewportSize.x);

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

	dest.mViewportSize.x = (uint16_t)mScale.z;
	dest.mViewportSize.y = (uint16_t)mScale.y;
	dest.mViewportSize.z = (uint16_t)mScale.z;

	dest.mNextEscape = mNextEscape;
	dest.mCurrentEscape = mCurrentEscape;

	return true;
}

void LDoorDOMNode::PostProcess()
{
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		mWestSouthRoom = mapNodeLocked->GetRoomByIndex(mNextEscape);
		mEastNorthRoom = mapNodeLocked->GetRoomByIndex(mCurrentEscape);
	}
}

void LDoorDOMNode::PreProcess()
{
	if (auto wsRoomLocked = mWestSouthRoom.lock())
		mNextEscape = wsRoomLocked->GetRoomIndex();
	else
		mNextEscape = 0;
	if (auto enRoomLocked = mEastNorthRoom.lock())
		mCurrentEscape = enRoomLocked->GetRoomIndex();
	else
		mCurrentEscape = 0;
}

void LDoorDOMNode::AssignJmpIdAndIndex(std::vector<std::shared_ptr<LDoorDOMNode>> doors)
{
	mDoorEntryNumber = doors.size() + 1;
	mJmpId = -1;

	int32_t highestSeen = -1;

	// For every ID betwee 0 and the number of door nodes...
	for (int i = 0; i < doors.size(); i++)
	{
		// Iterate each door and see if it has i as an ID.
		bool bIsIDUsed = false;
		for (auto d : doors)
		{
			// We record the highest value we've seen
			// so we can have a valid ID even if there are no holes in the used IDs.
			if (d->GetJmpId() > highestSeen)
				highestSeen = d->GetJmpId();

			// We found a door node i as an ID, so stop iterating.
			if (d->GetJmpId() == i)
			{
				bIsIDUsed = true;
				break;
			}
		}

		// If this ID isn't used, we can use it!
		if (!bIsIDUsed)
		{
			mJmpId = i;
			break;
		}
	}

	// There were no holes in the used IDs, so we'll just
	// take the highest ID and add 1. That's our new unique ID!
	if (mJmpId == -1)
		mJmpId = highestSeen++;
}

bool LDoorDOMNode::HasRoomReference(std::shared_ptr<LRoomDOMNode> room)
{
	return mWestSouthRoom.lock() == room || mEastNorthRoom.lock() == room;
}
