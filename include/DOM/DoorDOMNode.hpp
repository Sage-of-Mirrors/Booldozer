#pragma once

#include "BGRenderDOMNode.hpp"

struct LStaticDoorData;

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
	Anteroom_Double_Door,
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

	int32_t mNextEscape;
	int32_t mCurrentEscape;

public:
	typedef LBGRenderDOMNode Super;

	LDoorDOMNode(std::string name);

	virtual void RenderDetailsUI(float dt) override;

	bool Load(const LStaticDoorData& source);
	bool Save(LStaticDoorData& dest);

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
