
#include "DOM/EventDataDOMNode.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

LEventDataDOMNode::LEventDataDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::EventData;
}

void LEventDataDOMNode::RenderDetailsUI(float dt)
{
    ImGuiTabBarFlags tabflags = ImGuiTabBarFlags_None;
    if(ImGui::BeginTabBar("EventModeTabs", tabflags)){
        if(ImGui::BeginTabItem("Script")){
            ImGui::InputTextMultiline("Script File", &mEventScript, {ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 50}, 0);
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