#include "DOM/UIRenderDOMNode.hpp"
#include "bigg.hpp"

LUIRenderDOMNode::LUIRenderDOMNode(std::string name) : LDOMNodeBase(name)
{
    mType = EDOMNodeType::UIRender;
}

void LUIRenderDOMNode::RenderUI(float dt)
{
    ImGui::Text(mName.c_str());
}
