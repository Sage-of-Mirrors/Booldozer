#pragma once

#include <imgui.h>
#include "DOM.hpp"
#include <magic_enum.hpp>
#include <type_traits>
#include <algorithm>
#include <json.hpp>

#include "IconsForkAwesome.h"

namespace LUIUtility
{
	// Renders a checkbox for the given boolean. Returns whether the checkbox was modified, in which case the bool pointer
	// now contains the new state.
	bool RenderCheckBox(bool* c);
	bool RenderCheckBox(std::string name, bool* c);
	// Renders a checkbox that toggles the given node's IsRendered state.
	void RenderCheckBox(LDOMNodeBase* node);

	// Renders an ImGui Selectable control and returns whether it was left-clicked.
	bool RenderNodeSelectable(LDOMNodeBase* node, const bool& highlight);
	void RenderNodeSelectableDragSource(LDOMNodeBase* node);

	bool RenderNodeSelectableTreeNode(const std::string& name, const bool& higlight, bool& selected);

	bool RenderComboBox(std::string name, std::vector<std::shared_ptr<LEntityDOMNode>> options, std::shared_ptr<LEntityDOMNode> current_selection);
	bool RenderComboBox(std::string name, std::map<std::string, std::string>& options, std::string& value);
	bool RenderComboBox(std::string name, nlohmann::ordered_json options, std::string& value);

	bool RenderTextInput(std::string name, std::string* value, const int width = 100);
	int TextInputCallback(ImGuiInputTextCallbackData* data);

	void RenderTransformUI(glm::mat4* transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

	void RenderTooltip(std::string tip);

	uint32_t RenderGizmoToggle();

	bool RenderFileDialog(const std::string& dialogName, std::string& outPath);

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
	bool RenderNodeReferenceCombo(std::string name, EDOMNodeType desiredType, std::weak_ptr<LDOMNodeBase> parent, std::weak_ptr<T>& currentReference)
	{
		if (parent.expired())
			return false;

		auto parentLock = parent.lock();
		auto referenceLocked = currentReference.lock();

		std::string previewName = referenceLocked != nullptr ? referenceLocked->GetName() : "[None]";
		bool changed = false;

		std::vector<std::shared_ptr<T>> candidates = parentLock->GetChildrenOfType<T>(desiredType);

		if (ImGui::BeginCombo(name.c_str(), previewName.c_str()))
		{
			// First, add a "None" option for nullptrs.
			ImGui::PushID(0);

			bool is_selected = (referenceLocked == nullptr);
			if (ImGui::Selectable("[None]", is_selected))
			{
				currentReference = std::weak_ptr<T>();
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
				bool is_selected = (referenceLocked == candidates[i]);
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

	template<typename T>
	bool RenderNodeReferenceVector(std::string name, EDOMNodeType desiredType, std::weak_ptr<LDOMNodeBase> parent, std::vector<std::weak_ptr<T>>& vector)
	{
		bool changed = false;

		ImGui::Text("%s",name.c_str());
		ImGui::SameLine();
		ImGui::Text(ICON_FK_PLUS_CIRCLE);
		if (ImGui::IsItemClicked(0))
		{
			vector.push_back(std::weak_ptr<T>());
		}
		ImGui::SameLine();
		ImGui::Text(ICON_FK_MINUS_CIRCLE);
		if (ImGui::IsItemClicked(0) && vector.size() > 0)
		{
			vector.erase(vector.end() - 1);
		}

		for (size_t i = 0; i < vector.size(); i++)
		{
			std::weak_ptr<T> element = vector[i];

			ImGui::PushID(i);
			if (RenderNodeReferenceCombo("", desiredType, parent, element))
			{
				vector[i] = element;
				changed = true;
			}
			ImGui::PopID();
		}

		return changed;
	}
};

namespace ImGui {    
    bool BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
	bool Spinner(const char* label, float radius, int thickness, const ImU32& color);
}