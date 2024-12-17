#include "ResUtil.hpp"
#include "Options.hpp"
#include "ui/BloEditor.hpp"
#include "imgui.h"
#include <map>
#include <format>

namespace BloEditor {

std::map<std::string, std::tuple<std::filesystem::path, std::string, std::string>> Menus {
    { "Save Screen", {"Kawano/ENGLISH/res_save.szp", "save_2.blo", "timg"}},
    { "File Select", {"Kawano/ENGLISH/res_slct.szp", "blo/file_select_1.blo", "timg"}},
    { "Options Menu", {"Kawano/ENGLISH/res_slct.szp", "blo/option_1.blo", "timg"}},
    { "Area Complete 0", {"Kawano/ENGLISH/res_acnt.szp", "blo/adjustment_0.blo", "timg"}},
    { "Area Complete 1", {"Kawano/ENGLISH/res_acnt.szp", "blo/adjustment_1.blo", "timg"}},
    { "Area Complete 2", {"Kawano/ENGLISH/res_acnt.szp", "blo/adjustment_2.blo", "timg"}},
    { "Controls Explantion", {"Kawano/ENGLISH/res_cont.szp", "controller_2.blo", "timg"}},
    { "GBH Treasure", {"game", "kawano/list/blo/sgb_1.blo", "kawano/base/timg"}},
    { "Map Screen", {"game", "kawano/base/blo/map_1.blo", "kawano/base/timg"}},
    { "GBH Scan Menu", {"game", "kawano/base/blo/gbf_1.blo", "kawano/base/timg"}},
    { "GBH Border Menu", {"game", "kawano/base/blo/gbf_0.blo", "kawano/base/timg"}},
    { "Hidden Mansion Star", {"game", "kawano/base/blo/star_1.blo", "kawano/base/timg"}}
};

bool BloEditorOpen { false };
std::string MenuSelected = "Save Screen";
std::shared_ptr<Blo::Screen> ScreenLoaded { nullptr };
std::shared_ptr<Blo::Pane> SelectedNode { nullptr };
std::shared_ptr<Blo::Pane> DraggingNode { nullptr };
ImVec2 DragStart {};

void Render(){
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x * 0.675f, ImGui::GetMainViewport()->Size.y * 0.78f}, ImGuiCond_Always);
	if (ImGui::BeginPopupModal("BloEditorTool", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiChildFlags_AlwaysAutoResize))
	{
		ImGui::Text("Menu Editor");
        ImGui::Separator();

        if(ScreenLoaded == nullptr){
            auto menuInfo = Menus["Save Screen"];
            ScreenLoaded = std::make_shared<Blo::Screen>();
            std::filesystem::path menuResPath = std::get<0>(menuInfo);
            std::string menuBloPath = std::get<1>(menuInfo);
            std::string menuTimgPath = std::get<2>(menuInfo);

            std::shared_ptr<Archive::Rarc> menuarc = Archive::Rarc::Create();
            bStream::CFileStream fstrm((std::filesystem::path(OPTIONS.mRootPath) / "files" / menuResPath).string(), bStream::Endianess::Big, bStream::OpenMode::In);
            if(menuarc->Load(&fstrm)){
                auto file = menuarc->GetFile(menuBloPath);
                auto timg = menuarc->GetFolder(menuTimgPath);
                bStream::CMemoryStream stream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                ScreenLoaded->Load(&stream, timg);
            }
        }

        if(ImGui::BeginCombo("##menuSelect", MenuSelected.c_str())){
            for(auto [menuName, menuInfo] : Menus){
                if(ImGui::Selectable(menuName.c_str())){
                    MenuSelected = menuName;
                    ScreenLoaded = std::make_shared<Blo::Screen>();
                    std::filesystem::path menuResPath = std::get<0>(menuInfo);
                    std::string menuBloPath = std::get<1>(menuInfo);
                    std::string menuTimgPath = std::get<2>(menuInfo);

                    if(menuResPath.string() == "game" && GCResourceManager.mLoadedGameArchive){
                        auto file = GCResourceManager.mGameArchive->GetFile(menuBloPath);
                        auto timg = GCResourceManager.mGameArchive->GetFolder(menuTimgPath);
                        bStream::CMemoryStream stream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                        ScreenLoaded->Load(&stream, timg);
                    } else if(menuResPath.string() != "game") {
                        std::shared_ptr<Archive::Rarc> menuarc = Archive::Rarc::Create();
                        bStream::CFileStream fstrm((std::filesystem::path(OPTIONS.mRootPath) / "files" / menuResPath).string(), bStream::Endianess::Big, bStream::OpenMode::In);
                        if(menuarc->Load(&fstrm)){
                            auto file = menuarc->GetFile(menuBloPath);
                            auto timg = menuarc->GetFolder(menuTimgPath);
                            bStream::CMemoryStream stream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                            ScreenLoaded->Load(&stream, timg);
                        }
                    }
                }
            }
            ImGui::EndCombo();
        }        

        ImGui::SameLine();
        
        auto style = ImGui::GetStyle();
        float saveButton = ImGui::CalcTextSize("Save").x + style.FramePadding.x * 2.0f;
        float closeButton = ImGui::CalcTextSize("Close").x + style.FramePadding.x * 2.0f;
        float widthNeeded = saveButton + style.ItemSpacing.x + closeButton;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);

        if(ImGui::Button("Save")){
            auto menuInfo = Menus[MenuSelected];
            std::filesystem::path menuResPath = std::get<0>(menuInfo);
            std::string menuBloPath = std::get<1>(menuInfo);
            std::string menuTimgPath = std::get<2>(menuInfo);

            if(menuResPath.string() == "game" && GCResourceManager.mLoadedGameArchive){
                auto file = GCResourceManager.mGameArchive->GetFile(menuBloPath);
                auto timg = GCResourceManager.mGameArchive->GetFolder(menuTimgPath);
                bStream::CMemoryStream stream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                ScreenLoaded->Save(&stream);
            } else if(menuResPath.string() != "game") {
                std::shared_ptr<Archive::Rarc> menuarc = Archive::Rarc::Create();
                bStream::CFileStream fstrm((std::filesystem::path(OPTIONS.mRootPath) / "files" / menuResPath).string(), bStream::Endianess::Big, bStream::OpenMode::In);
                if(menuarc->Load(&fstrm)){
                    auto file = menuarc->GetFile(menuBloPath);
                    auto timg = menuarc->GetFolder(menuTimgPath);
                    bStream::CMemoryStream stream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                    ScreenLoaded->Save(&stream);
                }
            }

        }

        ImGui::SameLine();

        if(ImGui::Button("Close")){
            ImGui::CloseCurrentPopup();
        }

        ImGui::Separator();

        auto blopos = ImGui::GetCursorScreenPos();
        if(ScreenLoaded != nullptr){
            ScreenLoaded->Draw(DraggingNode);
            if(DraggingNode != nullptr && DragStart.x == 0 && DragStart.y == 0){
                DragStart = {DraggingNode->mRect[0], DraggingNode->mRect[1]};
            }
            ImGui::SameLine();
            ImGui::BeginChild("##leftHandWindow");
            ImGui::BeginChild("##hierarchy", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y*0.5f});
            ScreenLoaded->DrawHierarchy(SelectedNode);
            ImGui::EndChild();
            ImGui::Text("Selected");
            ImGui::Separator();
            if(SelectedNode != nullptr){
                if(SelectedNode->Type() == Blo::ElementType::Picture){
                    ImGui::Text(std::format("Texture Path {}", std::reinterpret_pointer_cast<Blo::Picture>(SelectedNode)->GetTexture()->mPath).c_str());
                }
            }
            ImGui::EndChild();
        }

        if(DraggingNode != nullptr){
            auto delta = ImGui::GetMouseDragDelta();
            DraggingNode->mRect[0] = DragStart.x + delta.x;
            DraggingNode->mRect[1] = DragStart.y + delta.y;
            if(ImGui::IsMouseReleased(0)){
                DraggingNode = nullptr;
                DragStart = {0, 0};
            }
        }

		ImGui::EndPopup();
	}
}

}