#include "DOM/RoomDataDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "io/StaticMapDataIO.hpp"

LRoomDataDOMNode::LRoomDataDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::RoomData;
}

void LRoomDataDOMNode::RenderDetailsUI(float dt)
{

}

bool LRoomDataDOMNode::Load(const uint32_t& index, const LStaticMapDataIO& source, const std::vector<std::shared_ptr<LRoomDOMNode>>& mapRooms, const std::vector<std::shared_ptr<LDoorDOMNode>>& mapDoors)
{
	// Room data
	LStaticRoomData roomData;
	source.GetRoomData(index, roomData);

	mCameraPositionIndex = roomData.mCameraPositionIndex;
	mFloor = roomData.mFloor;
	mDoorZone = roomData.mDoorZone;
	mRoomID = roomData.mRoomID;

	mCameraBehavior = roomData.mCameraBehavior;

	DeconstructBoundingBox(roomData.mBoundingBoxMin, roomData.mBoundingBoxMax);

	mUnknown1 = roomData.mUnknown1;
	mUnknown2 = roomData.mUnknown2;

	mDarkColor.r = roomData.mDarkColor.r / 255.f;
	mDarkColor.g = roomData.mDarkColor.g / 255.f;
	mDarkColor.b = roomData.mDarkColor.b / 255.f;
	mDarkColor.a = roomData.mDarkColor.a / 255.f;

	// Resource path
	source.GetRoomResourcePath(mCameraPositionIndex, mResourcePath);

	// Doors
	source.GetDoorListData(roomData.mDoorListIndex, mDoorListIndices);

	for (uint16_t dIndex : mDoorListIndices)
		mDoorList.push_back(mapDoors[dIndex]);

	// Adjacent rooms
	source.GetAdjacentRoomListData(mCameraPositionIndex, mAdjacentRoomIndices);

	for (uint16_t rIndex : mAdjacentRoomIndices)
		mAdjacentRooms.push_back(mapRooms[rIndex]);

	return true;
}

bool LRoomDataDOMNode::Save(LStaticRoomData& dest)
{
	dest.mCameraPositionIndex = mCameraPositionIndex;
	dest.mFloor = mFloor;
	dest.mDoorZone = mDoorZone;
	dest.mRoomID = mRoomID;

	dest.mCameraBehavior = mCameraBehavior;

	glm::vec3 bmin, bmax;
	ConstructBoundingBox(bmin, bmax);

	dest.mBoundingBoxMin.x = bmin.z;
	dest.mBoundingBoxMin.y = bmin.y;
	dest.mBoundingBoxMin.z = bmin.x;

	dest.mBoundingBoxMax.x = bmax.z;
	dest.mBoundingBoxMax.y = bmax.y;
	dest.mBoundingBoxMax.z = bmax.x;

	dest.mUnknown1 = mUnknown1;
	dest.mUnknown2 = mUnknown2;

	dest.mDarkColor.r = mDarkColor.r * 255;
	dest.mDarkColor.g = mDarkColor.g * 255;
	dest.mDarkColor.b = mDarkColor.b * 255;
	dest.mDarkColor.a = mDarkColor.a * 255;

	return true;
}

void LRoomDataDOMNode::DeconstructBoundingBox(const glm::vec3& min, const glm::vec3& max)
{
	mScale = (max - min) * 0.5f;
	float t = mScale.x;
	mScale.x = mScale.z;
	mScale.z = t;

	mPosition = min + (max - min) * 0.5f;// + (mScale);
	float temp = mPosition.x;
	mPosition.x = mPosition.z;
	mPosition.z = temp;
}

void LRoomDataDOMNode::ConstructBoundingBox(glm::vec3& min, glm::vec3& max)
{
	glm::vec3 halfExtents = mScale; //* 2.f;

	min = mPosition - halfExtents;
	max = mPosition + halfExtents;
}

bool LRoomDataDOMNode::CheckPointInBounds(const glm::vec3& point)
{
	glm::vec3 min, max;
	ConstructBoundingBox(min, max);

	bool xBounds = (point.x >= min.x) && (point.x < max.x);
	bool yBounds = (point.y >= min.y) && (point.y < max.y);
	bool zBounds = (point.z >= min.z) && (point.z < max.z);

	return xBounds && yBounds && zBounds;
}
