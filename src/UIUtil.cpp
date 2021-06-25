#include "UIUtil.hpp"

bool LUIUtility::RenderCheckBox(std::string name, bool* c)
{
    if (ImGui::Checkbox(name.c_str(), c))
        return true;

    return false;
}

void LUIUtility::RenderCheckBox(LDOMNodeBase* node)
{
    bool c = node->GetIsRendered();

    if (RenderCheckBox("##is_rendered", &c))
        node->SetIsRendered(c);
}

bool LUIUtility::RenderNodeSelectable(LDOMNodeBase* node)
{
    bool s = node->GetIsSelected();

    ImGui::Selectable(node->GetName().c_str(), &s);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        return true;
    }
    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("###node_context_menu");
        return true;
    }

    return false;
}
