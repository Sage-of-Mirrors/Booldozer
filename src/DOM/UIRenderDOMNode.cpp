#include "DOM/UIRenderDOMNode.hpp"
#include "UIUtil.hpp"

LUIRenderDOMNode::LUIRenderDOMNode(std::string name) : LDOMNodeBase(name)
{
    mType = EDOMNodeType::UIRender;
}

void LUIRenderDOMNode::RenderHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection)
{
    if (LUIUtility::RenderNodeSelectable(this, self->GetIsSelected()))
    {
        mode_selection->AddToSelection(self);
    }
}

void LUIRenderDOMNode::RenderDetailsUI(float dt)
{
    ImGui::Text("%s", mName.c_str());
}
