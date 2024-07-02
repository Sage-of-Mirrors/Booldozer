
#include "DOM/EventDataDOMNode.hpp"
#include "DOM/CameraAnimationDOMNode.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "fmt/core.h"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include <regex>

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

void LEventDataDOMNode::RenderDetailsUI(float dt, TextEditor* event)
{
    ImGuiTabBarFlags tabflags = ImGuiTabBarFlags_None;
    if(ImGui::BeginTabBar("EventModeTabs", tabflags)){
        if(ImGui::BeginTabItem("Script")){
            //ImGui::InputTextMultiline("Script File", &mEventScript, {ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 50}, 0);
            ImVec2 cRegion = ImGui::GetContentRegionAvail();
            cRegion.y -= 25;
            event->Render("Event Script", cRegion, true);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Text")){
            int num = 0;
            for(std::vector<std::string>::iterator str = mEventText.begin(); str != mEventText.end(); str++){
                ImGui::InputTextMultiline(fmt::format("##msg{}", num++).c_str(), &(*str));

                ImGui::SameLine();

                ImGui::Text(ICON_FK_MINUS_CIRCLE);
                if(ImGui::IsItemClicked()){
                    mEventText.erase(str);
                }
            }

            ImGui::Text(ICON_FK_PLUS_CIRCLE);
            if(ImGui::IsItemClicked()){
                mEventText.push_back("");
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
        std::string messages = std::string((char*)msgFile->GetData(), msgFile->GetSize());
        std::string msg = "";
        for(std::string::iterator c = messages.begin(); c < messages.end(); c++){
            if(*c == '\n' && c != messages.begin() && *(c-1) == '\r'){
                mEventText.push_back( LGenUtility::SjisToUtf8(msg));
                msg = "";
            } else {
                msg += (*c);
            }
        }
    }

    if(txtFile != nullptr){
        mEventScript = LGenUtility::SjisToUtf8(std::string((char*)txtFile->GetData(), txtFile->GetSize()));
    }

}

void LEventDataDOMNode::SaveEventArchive(){
    std::shared_ptr<Archive::File> msgFile = mEventArchive->GetFile(std::filesystem::path("message") / std::string(mEventMessagePath + ".csv"));
    std::shared_ptr<Archive::File> txtFile = mEventArchive->GetFile(std::filesystem::path("text") / std::string(mEventScriptPath + ".txt"));


    std::string msg = "";
    for(auto str : mEventText){
        msg += str;
        msg += "\r\n";
    }

    std::string msgFileData = LGenUtility::Utf8ToSjis(msg);
    
    std::string txtFileData = LGenUtility::Utf8ToSjis(mEventScript);
    
    if(msgFile != nullptr){
        msgFile->SetData((uint8_t*)msgFileData.data(), msgFileData.size());
    } else {
        msgFile = Archive::File::Create();
        msgFile->SetName(std::string(mEventMessagePath + ".csv"));
        msgFile->SetData((uint8_t*)msgFileData.data(), msgFileData.size());
        if(mEventArchive->GetFolder("message") != nullptr){
           mEventArchive->GetFolder("message")->AddFile(msgFile);
        } else {
            std::shared_ptr<Archive::Folder> msgDir = Archive::Folder::Create(mEventArchive);
            msgDir->SetName("message");
            msgDir->AddFile(msgFile);
            mEventArchive->GetRoot()->AddSubdirectory(msgDir);
        }
    }

    if(txtFile != nullptr){
        txtFile->SetData((uint8_t*)txtFileData.data(), txtFileData.size());
    } else {
        txtFile = Archive::File::Create();
        txtFile->SetName(std::string(mEventScriptPath + ".txt"));
        txtFile->SetData((uint8_t*)txtFileData.data(), txtFileData.size());
        if(mEventArchive->GetFolder("text") != nullptr){
           mEventArchive->GetFolder("text")->AddFile(txtFile);
        } else {
            std::shared_ptr<Archive::Folder> txtDir = Archive::Folder::Create(mEventArchive);
            txtDir->SetName("text");
            txtDir->AddFile(txtFile);
            mEventArchive->GetRoot()->AddSubdirectory(txtDir);
        }
    }

    std::cout << "[EventDOMNode]: Writing event to " << mEventPath << std::endl;
    mEventArchive->SaveToFile(mEventPath, Compression::Format::YAY0);
}