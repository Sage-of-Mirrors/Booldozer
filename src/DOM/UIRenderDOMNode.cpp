#include "DOM/UIRenderDOMNode.hpp"
#include "UIUtil.hpp"

LUIRenderDOMNode::LUIRenderDOMNode(std::string name) : LDOMNodeBase(name)
{
    mType = EDOMNodeType::UIRender;
}

void LUIRenderDOMNode::RenderHierarchyUI(float dt)
{
    if (LUIUtility::RenderNodeSelectable(this))
    {
        //ModeSelection->ModifySelection(this);
    }
}

void LUIRenderDOMNode::RenderDetailsUI(float dt)
{
    ImGui::Text(mName.c_str());
}
