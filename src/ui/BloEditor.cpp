#include "ResUtil.hpp"
#include "Options.hpp"
#include "ui/BloEditor.hpp"
#include "imgui.h"
#include <map>

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
    { "GBH Scan Menu", {"game", "kawano/base/blo/gbf_0.blo", "kawano/base/timg"}},
    { "GBH Border Menu", {"game", "kawano/base/blo/gbf_1.blo", "kawano/base/timg"}},
    { "Hidden Mansion Star", {"game", "kawano/base/blo/star_1.blo", "kawano/base/timg"}}
};

bool BloEditorOpen { false };
std::string MenuSelected = "Save Screen";
std::shared_ptr<Blo::Screen> ScreenLoaded { nullptr };

void Render(){
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x * 0.85f, ImGui::GetMainViewport()->Size.y * 0.75f}, ImGuiCond_Always);
	if (ImGui::BeginPopupModal("BloEditorTool", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiChildFlags_AlwaysAutoResize))
	{
		ImGui::Text("Menu Editor");
        ImGui::SameLine();

        if(ImGui::BeginCombo("##menuSelect", MenuSelected.c_str())){
            for(auto [menuName, menuInfo] : Menus){
                if(ImGui::Selectable(menuName.c_str())){
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

        ImGui::Separator();

        if(ScreenLoaded != nullptr){
            ScreenLoaded->Draw();
            ImGui::SameLine();
            ScreenLoaded->DrawHierarchy();

            if(ImGui::Button("Close")){
                ImGui::CloseCurrentPopup();
            }
        }

		ImGui::EndPopup();
	}
}

}