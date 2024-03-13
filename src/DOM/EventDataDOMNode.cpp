
#include "DOM/EventDataDOMNode.hpp"
#include "DOM/CameraAnimationDOMNode.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "fmt/core.h"
#include "UIUtil.hpp"
#include "GenUtil.hpp"

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
                ImGui::InputText(fmt::format("Speak {0}", i).c_str(), &mEventText[i], 0);
            }

            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
}

void LEventDataDOMNode::LoadEventArchive(std::shared_ptr<Archive::Rarc> arc, std::filesystem::path eventPath, std::string eventScriptName, std::string eventCsvName){
    mEventArchive = arc;

    mEventPath = eventPath;
    mEventMessagePath = eventCsvName;
    mEventScriptPath = eventScriptName;

    std::shared_ptr<Archive::File> msgFile = mEventArchive->GetFile(std::filesystem::path("message") / std::string(eventCsvName + ".csv"));
    std::shared_ptr<Archive::File> txtFile = mEventArchive->GetFile(std::filesystem::path("text") / std::string(eventScriptName + ".txt"));

    if(msgFile != nullptr){
        mEventText = {};

        std::stringstream msgFileFull = std::stringstream(LGenUtility::SjisToUtf8(std::string((char*)msgFile->GetData(), msgFile->GetSize())));
        std::string msg = "";
        while(!std::getline(msgFileFull, msg).eof()){
            mEventText.push_back(msg);
        }
    }

    if(txtFile != nullptr){
        mEventScript = LGenUtility::SjisToUtf8(std::string((char*)txtFile->GetData(), txtFile->GetSize()));
    }

}

void LEventDataDOMNode::SaveEventArchive(){
    std::shared_ptr<Archive::File> msgFile = mEventArchive->GetFile(std::filesystem::path("message") / std::string(mEventMessagePath + ".csv"));
    std::shared_ptr<Archive::File> txtFile = mEventArchive->GetFile(std::filesystem::path("text") / std::string(mEventScriptPath + ".txt"));

    if(msgFile != nullptr){
        std::string writeStr = "";
        for (auto msg : mEventText){
            writeStr.append(LGenUtility::Utf8ToSjis(msg)+"\n");
        }
        msgFile->SetData((uint8_t*)writeStr.data(), writeStr.size());
    }

    if(txtFile != nullptr){
        std::string writeStr = LGenUtility::Utf8ToSjis(mEventScript);
        txtFile->SetData((uint8_t*)writeStr.data(), writeStr.size());
    }

    mEventArchive->SaveToFile(mEventPath, Compression::Format::YAY0);
}