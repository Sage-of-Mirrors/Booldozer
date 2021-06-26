#include "DOM/UIRenderDOMNode.hpp"
#include "UIUtil.hpp"

LUIRenderDOMNode::LUIRenderDOMNode(std::string name) : LDOMNodeBase(name)
{
    mType = EDOMNodeType::UIRender;
}

void LUIRenderDOMNode::RenderHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection)
{
    if (LUIUtility::RenderNodeSelectable(this))
    {
        mode_selection->AddToSelection(self);
    }
}

void LUIRenderDOMNode::RenderDetailsUI(float dt)
{
    ImGui::Text(mName.c_str());
}
