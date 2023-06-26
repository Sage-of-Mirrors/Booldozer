#pragma once

#include "BGRenderDOMNode.hpp"
#include "glm/glm.hpp"

class LStaticMapDataIO;
struct LStaticRoomData;
class LDoorDOMNode;
class LRoomDOMNode;

class LRoomDataDOMNode : public LBGRenderDOMNode
{
	uint8_t mRoomIndex;
	int8_t mFloor;
	uint8_t mDoorZone;
	uint8_t mRoomID;

	uint32_t mCameraBehavior;

	uint32_t mUnknown1;
	uint32_t mUnknown2;

	std::vector<uint16_t> mDoorListIndices;
	std::vector<uint16_t> mAdjacentRoomIndices;

	glm::vec4 mDarkColor;

	std::vector<std::weak_ptr<LDoorDOMNode>> mDoorList;
	std::vector<std::weak_ptr<LRoomDOMNode>> mAdjacentRooms;

	std::string mResourcePath;
	std::vector<std::string> mAltResPaths;

	glm::vec3 bbmin, bbmax;

	void DeconstructBoundingBox(const glm::vec3& min, const glm::vec3& max);
	void ConstructBoundingBox(glm::vec3& min, glm::vec3& max);

public:
	typedef LBGRenderDOMNode Super;

	LRoomDataDOMNode(std::string name);

	std::string GetResourcePath() { return mResourcePath; }
	std::vector<std::string> GetAltResourcePaths() { return mAltResPaths; }
	std::string GetAltResourcePath(uint32_t index)
	{
		if (index < 0 || index >= mAltResPaths.size())
			return mResourcePath;

		return mAltResPaths[index];
	}

	std::vector<std::weak_ptr<LRoomDOMNode>>& GetAdjacencyList() { return mAdjacentRooms; }
	std::vector<std::weak_ptr<LDoorDOMNode>> GetDoorList() { return mDoorList; }

	glm::vec3 GetMin() { return bbmin; }
	glm::vec3 GetMax() { return bbmax; }

	void SetMin(glm::vec3 min) { bbmin = min; }
	void SetMax(glm::vec3 max) { bbmax = max; }

	float* GetDarkColor() { return &mDarkColor.r; }

	int32_t GetRoomIndex() const { return mRoomIndex; }
	int32_t GetRoomID() const { return mRoomID; }

	void RenderTransformUI();

	virtual void RenderDetailsUI(float dt) override;

	bool Load(const uint32_t& index, const LStaticMapDataIO& source, const std::vector<std::shared_ptr<LRoomDOMNode>>& mapRooms, const std::vector<std::shared_ptr<LDoorDOMNode>>& mapDoors);
	bool Save(LStaticRoomData& dest);

	bool CheckPointInBounds(const glm::vec3& point);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::RoomData)
			return true;

		return Super::IsNodeType(type);
	}
};
