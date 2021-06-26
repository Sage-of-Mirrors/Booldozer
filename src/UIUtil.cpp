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

bool LUIUtility::RenderComboBox(std::string name, std::vector<std::shared_ptr<LEntityDOMNode>> options, std::shared_ptr<LEntityDOMNode> current_selection)
{
	bool changed = false;

	if (ImGui::BeginCombo(name.c_str(), ""))
	{
		for (uint32_t i = 0; i < options.size(); i++)
		{
			ImGui::PushID(i);

			bool is_selected = (current_selection == options[i]);
			if (ImGui::Selectable(options[i]->GetName().c_str(), is_selected))
			{
				current_selection = options[i];
				changed = true;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();

			ImGui::PopID();
		}

		ImGui::EndCombo();
	}

	return changed;
}
