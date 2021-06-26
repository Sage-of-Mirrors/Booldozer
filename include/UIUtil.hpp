#pragma once

#include "imgui.h"
#include "DOM.hpp"
#include "../lib/magic_enum/include/magic_enum.hpp"
#include <type_traits>

namespace LUIUtility
{
	// Renders a checkbox for the given boolean. Returns whether the checkbox was modified, in which case the bool pointer
	// now contains the new state.
	bool RenderCheckBox(std::string name, bool* c);
	// Renders a checkbox that toggles the given node's IsRendered state.
	void RenderCheckBox(LDOMNodeBase* node);

	// Renders an ImGui Selectable control and returns whether it was left-clicked.
	bool RenderNodeSelectable(LDOMNodeBase* node);

	bool RenderComboBox(std::string name, std::vector<std::shared_ptr<LEntityDOMNode>> options, std::shared_ptr<LEntityDOMNode> current_selection);

	template<typename T>
	bool RenderComboEnum(std::string name, T& current_value)
	{
		static_assert(std::is_enum_v<T>, "T must be an enum!");

		auto& names = magic_enum::enum_names<T>();
		bool changed = false;

		if (ImGui::BeginCombo(name.c_str(), magic_enum::enum_name(current_value).data()))
		{
			for (uint32_t i = 0; i < magic_enum::enum_count<T>(); i++)
			{
				ImGui::PushID(i);

				bool is_selected = (current_value == magic_enum::enum_index<T>((T)i));
				if (ImGui::Selectable(names[i].data(), is_selected))
				{
					current_value = magic_enum::enum_cast<T>(names[i]).value();
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
};
