#include "DOM/MapCollisionDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "imgui.h"
#include "../lib/bStream/bstream.h"

LMapCollisionDOMNode::LMapCollisionDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::MapCollision;
}

std::string LMapCollisionDOMNode::GetName()
{
	return "Collision"; //uh
}

void LMapCollisionDOMNode::RenderDetailsUI(float dt)
{

}

bool LMapCollisionDOMNode::Load(bStream::CMemoryStream* stream)
{
    //??
	return false;
}

bool LMapCollisionDOMNode::Save(bStream::CMemoryStream* stream)
{
	return false;
}

void LMapCollisionDOMNode::PostProcess()
{

}

void LMapCollisionDOMNode::PreProcess()
{

}
