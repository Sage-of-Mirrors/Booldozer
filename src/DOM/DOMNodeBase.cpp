#include "DOM/DOMNodeBase.hpp"

std::vector<std::shared_ptr<LDOMNodeBase>> LDOMNodeBase::GetChildrenOfType(EDOMNodeType type)
{
	std::vector<std::shared_ptr<LDOMNodeBase>> matchingNodes;

	GatherChildrenOfType(matchingNodes, type);

	return matchingNodes;
}

void LDOMNodeBase::GatherChildrenOfType(std::vector<std::shared_ptr<LDOMNodeBase>>& list, EDOMNodeType type)
{
	for (std::shared_ptr<LDOMNodeBase> child : Children)
	{
		if (child->IsNodeType(type))
			list.push_back(child);

		child->GatherChildrenOfType(list, type);
	}
}
