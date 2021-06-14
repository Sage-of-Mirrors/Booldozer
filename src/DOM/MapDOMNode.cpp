#include "DOM/MapDOMNode.hpp"

LMapDOMNode::LMapDOMNode(std::string name) : LDOMNodeBase(name)
{
	mType = EDOMNodeType::Map;
}
