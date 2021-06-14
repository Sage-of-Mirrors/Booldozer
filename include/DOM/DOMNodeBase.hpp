#pragma once

#include <vector>
#include <memory>
#include <string>

enum EDOMNodeState
{
	EDOMNodeState_Expanded = 0x01,
	EDOMNodeState_Rendered = 0x02,
	EDOMNodeState_Selected = 0x04
};

enum class EDOMNodeType
{
	Base,
	UIRender,
	BGRender,
	Map,
	Room,

	// Entities
	Entity,
	Furniture,
};

// Base class for all DOM (Document Object Model) nodes.
class LDOMNodeBase
{
	uint32_t mNodeState;

protected:
	EDOMNodeType mType;
	std::string mName;

	void GatherChildrenOfType(std::vector<std::shared_ptr<LDOMNodeBase>>& list, EDOMNodeType type);

public:
	LDOMNodeBase(std::string name) { mName = name; }

	std::vector<std::shared_ptr<LDOMNodeBase>> Children;

/*=== Node state ===*/
	bool GetIsExpanded() { return (mNodeState & EDOMNodeState_Expanded) != 0; }
	bool GetIsRendered() { return (mNodeState & EDOMNodeState_Rendered) != 0; }
	bool GetIsSelected() { return (mNodeState & EDOMNodeState_Selected) != 0; }

	void SetIsExpanded(bool expanded)
	{
		if (expanded)
			mNodeState |= EDOMNodeState_Expanded;
		else
			mNodeState &= ~EDOMNodeState_Expanded;
	}

	void SetIsRendered(bool rendered)
	{
		if (rendered)
			mNodeState |= EDOMNodeState_Rendered;
		else
			mNodeState &= ~EDOMNodeState_Rendered;
	}

	void SetIsSelected(bool selected)
	{
		if (selected)
			mNodeState |= EDOMNodeState_Selected;
		else
			mNodeState &= ~EDOMNodeState_Selected;
	}

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const
	{
		return mType == type;
	}

	// Returns a vector of nodes, guaranteeing that they are at least of the given type.
	std::vector<std::shared_ptr<LDOMNodeBase>> GetChildrenOfType(EDOMNodeType type);
};
