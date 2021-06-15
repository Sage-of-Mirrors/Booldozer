#include "DOM/DOMNodeBase.hpp"

template<typename T>
std::vector<std::shared_ptr<T>> LDOMNodeBase::GetChildrenOfType(EDOMNodeType type)
{
	std::vector<std::shared_ptr<T>> matchingNodes;

	GatherChildrenOfType(matchingNodes, type);

	return matchingNodes;
}

template<typename T>
void LDOMNodeBase::GatherChildrenOfType(std::vector<std::shared_ptr<T>>& list, EDOMNodeType type)
{
	for (std::shared_ptr<T> child : Children)
	{
		if (child->IsNodeType(type))
			list.push_back(std::static_pointer_cast<T>(child));

		child->GatherChildrenOfType(list, type);
	}
}
