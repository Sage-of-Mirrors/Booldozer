#pragma once

#include "BGRenderDOMNode.hpp"
#include "glm/glm.hpp"

class LStaticMapDataIO;
class LDoorDOMNode;
class LRoomDOMNode;

class LRoomDataDOMNode : public LBGRenderDOMNode
{
	uint8_t mCameraPositionIndex;
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

	void DeconstructBoundingBox(const glm::vec3& min, const glm::vec3& max);
	void ConstructBoundingBox(glm::vec3& min, glm::vec3& max);

public:
	typedef LBGRenderDOMNode Super;

	LRoomDataDOMNode(std::string name);

	std::string GetResourcePath() { return mResourcePath; }
	std::vector<std::weak_ptr<LRoomDOMNode>> GetAdjacencyList() { return mAdjacentRooms; }

	virtual void RenderDetailsUI(float dt) override;

	bool Load(const uint32_t& index, const LStaticMapDataIO& source, const std::vector<std::shared_ptr<LRoomDOMNode>>& mapRooms, const std::vector<std::shared_ptr<LDoorDOMNode>>& mapDoors);
	bool Save(LStaticMapDataIO& dest);

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
