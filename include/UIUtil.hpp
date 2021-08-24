#pragma once

#include "imgui.h"
#include "DOM.hpp"
#include "../lib/magic_enum/include/magic_enum.hpp"
#include <type_traits>
#include <algorithm>

namespace LUIUtility
{
	// Renders a checkbox for the given boolean. Returns whether the checkbox was modified, in which case the bool pointer
	// now contains the new state.
	bool RenderCheckBox(std::string name, bool* c);
	// Renders a checkbox that toggles the given node's IsRendered state.
	void RenderCheckBox(LDOMNodeBase* node);

	// Renders an ImGui Selectable control and returns whether it was left-clicked.
	bool RenderNodeSelectable(LDOMNodeBase* node);
	void RenderNodeSelectableDragSource(LDOMNodeBase* node);

	bool RenderComboBox(std::string name, std::vector<std::shared_ptr<LEntityDOMNode>> options, std::shared_ptr<LEntityDOMNode> current_selection);
	bool RenderComboBox(std::string name, std::map<std::string, std::string>& options, std::string& value);

	bool RenderTextInput(std::string name, std::string* value);
	int TextInputCallback(ImGuiInputTextCallbackData* data);

	void RenderTransformUI(glm::mat4* transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

	void RenderTooltip(std::string tip);

	uint32_t RenderGizmoToggle();

	// Renders a combobox for the given enum.
	template<typename T>
	bool RenderComboEnum(std::string name, T& current_value)
	{
		static_assert(std::is_enum_v<T>, "T must be an enum!");

		bool changed = false;
		auto origName = magic_enum::enum_name(current_value);
		std::string curName { origName.begin(), origName.end() };
		std::replace(curName.begin(), curName.end(), '_', ' ');

		// Combobox start
		if (ImGui::BeginCombo(name.c_str(), curName.c_str()))
		{
			// Iterating the possible enum values...
			for (auto [enum_value, enum_name] : magic_enum::enum_entries<T>())
			{
				std::string displayName { enum_name.begin(), enum_name.end() };
				std::replace(displayName.begin(), displayName.end(), '_', ' ');

				// ImGui ID stack is now at <previous value>##<enum value>
				ImGui::PushID(static_cast<int>(enum_value));

				// Render the combobox item for this enum value
				bool is_selected = (current_value == enum_value);
				if (ImGui::Selectable(displayName.c_str(), is_selected))
				{
					current_value = enum_value;
					changed = true;
				}

				// Set initial focus when opening the combo
				if (is_selected)
					ImGui::SetItemDefaultFocus();

				// ImGui ID stack returns to <previous value>
				ImGui::PopID();
			}

			// End combobox
			ImGui::EndCombo();
		}

		return changed;
	}

	// Renders a combobox allowing the user to pick from a list of nodes of type T, fetched from the given parent node.
	template<typename T>
	bool RenderNodeReferenceCombo(std::string name, EDOMNodeType desiredType, std::shared_ptr<LDOMNodeBase> parent, std::shared_ptr<T>& currentReference)
	{
		std::string previewName = currentReference != nullptr ? currentReference->GetName() : "[None]";

		bool changed = false;
		std::vector<std::shared_ptr<T>> candidates = parent->GetChildrenOfType<T>(desiredType);

		if (ImGui::BeginCombo(name.c_str(), previewName.c_str()))
		{
			// First, add a "None" option for nullptrs.
			ImGui::PushID(0);

			bool is_selected = (currentReference == nullptr);
			if (ImGui::Selectable("[None]", is_selected))
			{
				currentReference = nullptr;
				changed = true;
			}

			// Set initial focus when opening the combo
			if (is_selected)
				ImGui::SetItemDefaultFocus();

			ImGui::PopID();

			// Then fill the combobox with the collected nodes.
			for (uint32_t i = 0; i < candidates.size(); i++)
			{
				// ImGui ID stack is now at <previous value>##<i>
				ImGui::PushID(i + 1);

				// Render the combobox item for this node
				bool is_selected = (currentReference == candidates[i]);
				if (ImGui::Selectable(candidates[i]->GetName().c_str(), is_selected))
				{
					currentReference = candidates[i];
					changed = true;
				}

				// Set initial focus when opening the combo
				if (is_selected)
					ImGui::SetItemDefaultFocus();

				// ImGui ID stack returns to <previous value>
				ImGui::PopID();
			}

			ImGui::EndCombo();
		}

		return changed;
	}
};
