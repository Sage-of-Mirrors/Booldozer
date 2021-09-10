#pragma once

#include "BlackoutDOMNode.hpp"

enum class EConditionType : uint32_t;

enum class EKeyAppearType : uint32_t
{
	Normal,
	Unknown_1,
	Unknown_2,
	Normal_2,
	Drop_Down
};

class LKeyDOMNode : public LBlackoutDOMNode
{
/*=== JMP properties ===*/
	std::string mCreateName;
	std::string mCodeName;

	int32_t mOpenDoorNumber;
	int32_t mSpawnFlag;
	int32_t mDespawnFlag;

	EConditionType mCondType;
	EKeyAppearType mAppearType;

	bool mIsVisible;

public:
	typedef LBlackoutDOMNode Super;

	LKeyDOMNode(std::string name);
	LKeyDOMNode(std::string name, bool isBlackoutKey);

	virtual void RenderDetailsUI(float dt) override;

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Key)
			return true;

		return Super::IsNodeType(type);
	}
};
