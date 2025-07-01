#include "UIUtil.hpp"
#include <glm/gtx/matrix_decompose.hpp>
#include "GenUtil.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include <IconsForkAwesome.h>
#include <imgui_internal.h>

bool LUIUtility::RenderCheckBox(bool* c)
{
	if(*c){
		ImGui::Text(ICON_FK_EYE);
	} else {
		ImGui::Text(ICON_FK_EYE_SLASH);
	}

	if(ImGui::IsItemClicked()){
		*c = !(*c);
		return true;
	}

	return false;
}

bool LUIUtility::RenderCheckBox(std::string name, bool* c)
{
	if(*c){
		ImGui::Text(std::format("{} {}", ICON_FK_CHECK_CIRCLE_O, name).c_str());
	} else {
		ImGui::Text(std::format("{} {}", ICON_FK_CIRCLE_O, name).c_str());
	}

	if(ImGui::IsItemClicked()){
		*c = !(*c);
		return true;
	}

	return false;
}

void LUIUtility::RenderCheckBox(LDOMNodeBase* node)
{
    bool c = node->GetIsRendered();

	if(c){
		ImGui::Text(ICON_FK_EYE);
	} else {
		ImGui::Text(ICON_FK_EYE_SLASH);
	}

	if(ImGui::IsItemClicked()){
		node->SetIsRendered(!c);
	}
}

bool LUIUtility::RenderNodeSelectable(LDOMNodeBase* node, const bool& highlight)
{
    bool s = highlight;

    ImGui::Selectable(std::format("{}##{}", node->GetName(), node->GetID()).c_str(), &s);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        return true;
    }
    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
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

bool LUIUtility::RenderNodeSelectableTreeNode(const std::string& name, const bool& highlight, bool& selected)
{
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (highlight)
		base_flags |= ImGuiTreeNodeFlags_Selected;

	bool node_open = ImGui::TreeNodeEx(name.c_str(), base_flags);
	if (ImGui::IsItemClicked())
		selected = true;

	return node_open;
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
			std::string selectableName = std::format("{0}##{1}", visible_name, internal_name);
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

bool LUIUtility::RenderComboBox(std::string name, nlohmann::ordered_json options, std::string& value)
{
	bool changed = false;

	std::string previewName = "";
	for (auto& section : options.items())
	{
		for (auto& entry : section.value().items())
		{
			if (entry.key() == value)
			{
				previewName = entry.value().get<std::string>();
				break;
			}
		}
	}

	if (ImGui::BeginCombo(name.c_str(), previewName.c_str()))
	{
		for (nlohmann::ordered_json section : options)
		{
			for (auto entry : section.items())
			{
				std::string jKey = entry.key();
				std::string jValue = entry.value().get<std::string>();

				std::string selectableName = std::format("{0}##{1}", jValue,  jKey);
				bool is_selected = (value == jKey);

				if (ImGui::Selectable(selectableName.c_str(), is_selected))
				{
					value = jKey;
					changed = true;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::Separator();
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
		if (ImGui::InputFloat("X", &translation.x) || ImGui::InputFloat("Y", &translation.y) || ImGui::InputFloat("Z", &translation.z))
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
		ImGui::InputFloat("X", &scale.x);
		ImGui::InputFloat("Y", &scale.y);
		ImGui::InputFloat("Z", &scale.z);
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

// Loading spinners from https://github.com/ocornut/imgui/issues/1901

namespace ImGui {
    
    bool BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;
        
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;
        
        // Render
        const float circleStart = size.x * 0.7f;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;
        
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);
        
        const float t = g.Time;
        const float r = size.y / 2;
        const float speed = 1.5f;
        
        const float a = speed*0;
        const float b = speed*0.333f;
        const float c = speed*0.666f;
        
        const float o1 = (circleWidth+r) * (t+a - speed * (int)((t+a) / speed)) / speed;
        const float o2 = (circleWidth+r) * (t+b - speed * (int)((t+b) / speed)) / speed;
        const float o3 = (circleWidth+r) * (t+c - speed * (int)((t+c) / speed)) / speed;
        
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
		return true;
    }

    bool Spinner(const char* label, float radius, int thickness, const ImU32& color) {
        ImGuiWindow* window = GetCurrentWindow();
		if(window == nullptr) return false;

        if (window->SkipItems)
            return false;
        
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);
        
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;
        
        // Render
        window->DrawList->PathClear();
        
        int num_segments = 30;
        int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));
        
        const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
        const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

        const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);
        
        for (int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * radius,
                                                centre.y + ImSin(a+g.Time*8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);
		return true;
	}
    
}