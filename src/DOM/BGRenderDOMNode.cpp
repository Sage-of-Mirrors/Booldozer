#include "DOM/BGRenderDOMNode.hpp"
#include "bigg.hpp"

LBGRenderDOMNode::LBGRenderDOMNode(std::string name) : LUIRenderDOMNode(name)
{
	mType = EDOMNodeType::BGRender;
}

void LBGRenderDOMNode::RenderBG(float dt)
{

}
