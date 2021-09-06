#include "UIUtil.hpp"
#include <glm/gtx/matrix_decompose.hpp>
#include "GenUtil.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"

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

	RenderNodeSelectableDragSource(node);

    return false;
}

void LUIUtility::RenderNodeSelectableDragSource(LDOMNodeBase* node)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload(node->GetNodeTypeString(), &node, sizeof(LDOMNodeBase*));
		ImGui::Text("%s", node->GetName().c_str());
		ImGui::EndDragDropSource();
	}
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

bool LUIUtility::RenderComboBox(std::string name, std::map<std::string, std::string>& options, std::string& value)
{
	bool changed = false;

	if (ImGui::BeginCombo(name.c_str(), options[value].c_str()))
	{
		for (auto [ internal_name, visible_name ] : options)
		{
			std::string selectableName = LGenUtility::Format(visible_name, "##", internal_name);
			bool is_selected = (value == internal_name);

			if (ImGui::Selectable(selectableName.c_str(), is_selected))
			{
				value = internal_name;
				changed = true;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	return changed;
}

bool LUIUtility::RenderTextInput(std::string name, std::string* value, const int width)
{
	ImGui::PushItemWidth(width);

	bool result = ImGui::InputText(name.c_str(), value->data(), value->size() + 1, ImGuiInputTextFlags_CallbackResize, LUIUtility::TextInputCallback, value);

	ImGui::PopItemWidth();

	return result;
}

int LUIUtility::TextInputCallback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		std::string* str;

		str = static_cast<std::string*>(data->UserData);
		str->resize(static_cast<size_t>(data->BufTextLen));
		data->Buf = str->data();
	}

	return 0;
}

void LUIUtility::RenderTransformUI(glm::mat4* transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
{
	bool transChanged = false;
	glm::vec3 oldPos;
	glm::quat oldRot;
	glm::vec3 oldScale;
	glm::vec3 skew;
	glm::vec4 persp;

	glm::decompose(*transform, oldScale, oldRot, oldPos, skew, persp);

	if (ImGui::TreeNode("Transform"))
	{
		ImGui::PushID("##pos");
		ImGui::Indent();
		ImGui::LabelText("", "Position");
		ImGui::Indent();
		if (ImGui::InputFloat("X", &translation.z) || ImGui::InputFloat("Y", &translation.y) || ImGui::InputFloat("Z", &translation.x))
			transChanged = true;

		ImGui::Unindent();
		ImGui::Unindent();
		ImGui::PopID();

		ImGui::PushID("##rot");
		ImGui::Indent();
		ImGui::LabelText("", "Rotation");
		ImGui::Indent();
		ImGui::InputFloat("X", &rotation.x);
		ImGui::InputFloat("Y", &rotation.y);
		ImGui::InputFloat("Z", &rotation.z);
		ImGui::Unindent();
		ImGui::Unindent();
		ImGui::PopID();

		ImGui::PushID("##scale");
		ImGui::Indent();
		ImGui::LabelText("", "Scale");
		ImGui::Indent();
		ImGui::InputFloat("X", &scale.z);
		ImGui::InputFloat("Y", &scale.y);
		ImGui::InputFloat("Z", &scale.x);
		ImGui::Unindent();
		ImGui::Unindent();
		ImGui::PopID();

		ImGui::TreePop();
	}

	if (transChanged)
		(*transform) = glm::translate(*transform, translation - oldPos);
}

void LUIUtility::RenderTooltip(std::string tip)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(tip.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

uint32_t LUIUtility::RenderGizmoToggle()
{
	uint32_t t = 0;

	// Translate
    ImGui::SetNextWindowPos(ImVec2(380, 25));
    ImGui::SetNextWindowSize(ImVec2(60, 35));

    ImGui::Begin("transform gadget window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

    if (ImGui::Button("Trans"))
		t = 0;

    ImGui::End();

	// Rotate
    ImGui::SetNextWindowPos(ImVec2(445, 25));
    ImGui::SetNextWindowSize(ImVec2(60, 35));

    ImGui::Begin("rotate gadget window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

    if (ImGui::Button("Rot"))
		t = 1;

    ImGui::End();

	// Scale
    ImGui::SetNextWindowPos(ImVec2(510, 25));
    ImGui::SetNextWindowSize(ImVec2(60, 35));

    ImGui::Begin("scale gadget window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

    if (ImGui::Button("Scale"))
		t = 1;

    ImGui::End();

	return t;
}

bool LUIUtility::RenderFileDialog(const std::string& dialogKey, std::string& outPath)
{
	bool success = false;

	// Render file dialog for opening map
	if (ImGuiFileDialog::Instance()->IsOpened() && ImGuiFileDialog::Instance()->Display(dialogKey.c_str()))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			outPath = ImGuiFileDialog::Instance()->GetFilePathName();
			success = true;
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}

	return success;
}
