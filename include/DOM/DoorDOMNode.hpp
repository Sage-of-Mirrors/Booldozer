#pragma once

#include "BGRenderDOMNode.hpp"

struct LStaticDoorData;
class LRoomDOMNode;

enum class EDoorOrientation : uint8_t
{
	Front_Facing = 1,
	Side_Facing,
	No_Fade = 4
};

enum class EDoorType : uint8_t
{
	Door,
	Viewport,
	Window
};

enum class EDoorModel : uint8_t
{
	None,
	Square_Mansion_Door,
	Round_Topped_Mansion_Door,
	Parlor_Double_Door,
	Anteroom_Door,
	Lab_Door,
	Gallery_Door,
	Nursery_Door,
	Twins_Door,
	Wooden_Door,
	Basement_Hallway_Door,
	Hearts_Double_Door,
	Clubs_Door,
	Diamonds_Door,
	Spades_Door
};

class LDoorDOMNode : public LBGRenderDOMNode
{
	EDoorOrientation mOrientation;
	EDoorType mDoorType;
	int32_t mJmpId;
	EDoorModel mModel;
	int32_t mDoorEntryNumber;
	glm::vec3 mViewportSize;

	int32_t mNextEscape;
	int32_t mCurrentEscape;

	std::weak_ptr<LRoomDOMNode> mWestSouthRoom;
	std::weak_ptr<LRoomDOMNode> mEastNorthRoom;

public:
	typedef LBGRenderDOMNode Super;

	LDoorDOMNode(std::string name);

	virtual std::string GetName() override;
	virtual void RenderDetailsUI(float dt) override;

	bool Load(const LStaticDoorData& source);
	bool Save(LStaticDoorData& dest);

	void PostProcess();
	void PreProcess();

	void AssignJmpIdAndIndex(std::vector<std::shared_ptr<LDoorDOMNode>> doors);
	bool HasRoomReference(std::shared_ptr<LRoomDOMNode> room);
	std::pair<std::shared_ptr<LRoomDOMNode>, std::shared_ptr<LRoomDOMNode>> GetRoomReferences() { return { mWestSouthRoom.lock(), mEastNorthRoom.lock() }; }

	int32_t GetJmpId() const { return mJmpId; }
	int32_t GetIndex() const { return mDoorEntryNumber; }
	EDoorModel GetModel() { return mModel; }
	EDoorOrientation GetOrientation() { return mOrientation; }

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Door)
			return true;

		return Super::IsNodeType(type);
	}
};
