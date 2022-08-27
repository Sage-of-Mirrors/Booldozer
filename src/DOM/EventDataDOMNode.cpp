
#include "DOM/EventDataDOMNode.hpp"
#include "DOM/CameraAnimationDOMNode.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "UIUtil.hpp"

LEventDataDOMNode::LEventDataDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::EventData;
}

void LEventDataDOMNode::RenderHierarchyUI(std::shared_ptr<LEventDataDOMNode> self, LEditorSelection* mode_selection){
    bool treeSelected = false;
	bool treeOpened = LUIUtility::RenderNodeSelectableTreeNode(GetName(), GetIsSelected(), treeSelected);

    if(treeOpened){
        int i = 0;
        for(auto child : self->GetChildrenOfType<LCameraAnimationDOMNode>(EDOMNodeType::CameraAnim)){
            ImGui::Indent();
            ImGui::PushID(i++);
            
            child->RenderHierarchyUI(child, mode_selection);

            ImGui::Unindent();
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

	if (treeSelected)
	{
		mode_selection->AddToSelection(GetSharedPtr<LUIRenderDOMNode>(EDOMNodeType::UIRender));
	}
}

void LEventDataDOMNode::RenderDetailsUI(float dt, TextEditor* editor)
{
    ImGuiTabBarFlags tabflags = ImGuiTabBarFlags_None;
    if(ImGui::BeginTabBar("EventModeTabs", tabflags)){
        if(ImGui::BeginTabItem("Script")){
            //ImGui::InputTextMultiline("Script File", &mEventScript, {ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 50}, 0);
            editor->Render("TextEditor");
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Text")){

            for (int i = 0; i < mEventText.size(); i++){
                ImGui::InputText(LGenUtility::Format("Speak ", i).c_str(), &mEventText[i], 0);
            }

            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
}