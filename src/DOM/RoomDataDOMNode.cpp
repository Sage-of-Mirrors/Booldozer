#include "DOM/RoomDataDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "io/StaticMapDataIO.hpp"
#include "UIUtil.hpp"

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
	LStaticRoomData roomData {0};
	source.GetRoomData(index, roomData);

	mRoomIndex = roomData.mCameraPositionIndex;
	mFloor = roomData.mFloor;
	mDoorZone = roomData.mDoorZone;
	mRoomID = roomData.mRoomID;

	mCameraBehavior = roomData.mCameraBehavior;

	DeconstructBoundingBox(roomData.mBoundingBoxMin, roomData.mBoundingBoxMax);
	bbmin = {roomData.mBoundingBoxMin.z, roomData.mBoundingBoxMin.y, roomData.mBoundingBoxMin.x};
	bbmax = {roomData.mBoundingBoxMax.z, roomData.mBoundingBoxMax.y, roomData.mBoundingBoxMax.x};

	mUnknown1 = roomData.mUnknown1;
	mUnknown2 = roomData.mUnknown2;

	mDarkColor.r = roomData.mDarkColor.r / 255.f;
	mDarkColor.g = roomData.mDarkColor.g / 255.f;
	mDarkColor.b = roomData.mDarkColor.b / 255.f;
	mDarkColor.a = roomData.mDarkColor.a / 255.f;

	// Resource path
	source.GetRoomResourcePath(mRoomID, mResourcePath);

	// Doors
	source.GetDoorListData(roomData.mDoorListIndex, mDoorListIndices);

	for (uint16_t dIndex : mDoorListIndices)
		mDoorList.push_back(mapDoors[dIndex]);

	// Adjacent rooms
	source.GetAdjacentRoomListData(mRoomID, mAdjacentRoomIndices);

	for (uint16_t rIndex : mAdjacentRoomIndices){
		if(mapRooms.size() > rIndex && mapRooms[rIndex] != nullptr){
			mAdjacentRooms.push_back(mapRooms[rIndex]);
		}
	}

	mAltResPaths = source.GetAltResourceData(mRoomID);

	return true;
}

bool LRoomDataDOMNode::Save(LStaticRoomData& dest)
{
	dest.mCameraPositionIndex = (uint8_t)mRoomIndex;
	dest.mFloor = mFloor;
	dest.mDoorZone = mDoorZone;
	dest.mRoomID = (uint8_t)mRoomID;

	dest.mCameraBehavior = (uint32_t)mCameraBehavior;

	dest.mBoundingBoxMin.x = bbmin.z;
	dest.mBoundingBoxMin.y = bbmin.y;
	dest.mBoundingBoxMin.z = bbmin.x;

	dest.mBoundingBoxMax.x = bbmax.z;
	dest.mBoundingBoxMax.y = bbmax.y;
	dest.mBoundingBoxMax.z = bbmax.x;

	dest.mUnknown1 = mUnknown1;
	dest.mUnknown2 = mUnknown2;

	dest.mDarkColor.r = mDarkColor.r * 255;
	dest.mDarkColor.g = mDarkColor.g * 255;
	dest.mDarkColor.b = mDarkColor.b * 255;
	dest.mDarkColor.a = mDarkColor.a * 255;

	return true;
}

void LRoomDataDOMNode::RenderTransformUI(){
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);
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
	//glm::vec3 min, max;
	//ConstructBoundingBox(min, max);

	bool xBounds = (point.x >= bbmin.x) && (point.x < bbmax.x);
	bool yBounds = (point.y >= bbmin.y) && (point.y < bbmax.y);
	bool zBounds = (point.z >= bbmin.z) && (point.z < bbmax.z);

	return xBounds && yBounds && zBounds;
}

void LRoomDataDOMNode::RemoveAdjacent(std::shared_ptr<LRoomDOMNode> remove){
	for(std::vector<std::weak_ptr<LRoomDOMNode>>::iterator room = mAdjacentRooms.begin(); room != mAdjacentRooms.end(); room++){
		if(!room->expired() && room->lock() == remove){
			LGenUtility::Log << "[RoomDataDOMNode]: Removing adjacent room" << std::endl;
			mAdjacentRooms.erase(room, room+1);
			break;
		}
	}
}