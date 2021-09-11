#include "DOM/BlackoutDOMNode.hpp"

LBlackoutDOMNode::LBlackoutDOMNode(std::string name, bool isBlackoutEntity) : Super(name)
{
	bIsActiveDuringBlackout = isBlackoutEntity;
	mType = EDOMNodeType::Blackout;
}
