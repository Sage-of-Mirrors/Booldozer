#include "imgui.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "IconsForkAwesome.h"
#include "Options.hpp"
#include "ProjectManager.hpp"
#include <format>
#include <scene/EditorScene.hpp>
#include "UIUtil.hpp"
#include "GCM.hpp"

namespace ProjectManager {

bool JustClosed { false };
bool OpenNewRoot { false };
bool ShowNewProjectDialog { false };
nlohmann::json ProjectsJson {};
std::string NewProjectRootPath { "." };
std::string NewProjectRootName { "." };
std::map<std::string, std::string> ProjectNames {};
std::map<std::string, uint32_t> ProjectBanners {};

void ExtractFolderISO(std::shared_ptr<Disk::Folder> folder){
    std::filesystem::create_directory(folder->GetName());
    std::filesystem::current_path(std::filesystem::current_path() / folder->GetName());


    for(auto subdir : folder->GetSubdirectories()){
        ExtractFolderISO(subdir);
    }

    for(auto file : folder->GetFiles()){
        bStream::CFileStream extractFile(file->GetName(), bStream::Endianess::Big, bStream::OpenMode::Out);
        extractFile.writeBytes(file->GetData(), file->GetSize());
    }

    std::filesystem::current_path(std::filesystem::current_path().parent_path());
}

void Init(){
    LGenUtility::Log << "[Project Manager] Initializing" << std::endl;
    if(std::filesystem::exists(std::filesystem::current_path() / "res" / "projects.json")){
        std::ifstream projectsFileIn((std::filesystem::current_path() / "res" / "projects.json").string());
        projectsFileIn >> ProjectsJson;
        LGenUtility::Log << "[Project Manager] Projects loaded" << std::endl;
    } else {
        LGenUtility::Log << "[Project Manager] Projects json not found, creating" << std::endl;
        std::ofstream{std::filesystem::current_path() / "res" / "projects.json"} << "{\"projects\":[]}";
    }

    // Load Project Banners
    for(auto project : ProjectsJson["projects"]){
        uint32_t thumbId;
		glGenTextures(1, &thumbId);
		glBindTexture(GL_TEXTURE_2D, thumbId);

        if(ProjectBanners.contains(project.get<std::string>())){
            glDeleteTextures(1, &ProjectBanners[project.get<std::string>()]);
        }

        ProjectBanners[project.get<std::string>()] = thumbId;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        std::vector<uint8_t> bnrImgData(96*32*4);
        std::filesystem::path bannerPath(std::filesystem::path(project.get<std::string>()) / "files" / "opening.bnr");
        if(std::filesystem::exists(bannerPath)){
            bStream::CFileStream bnr(bannerPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
            bnr.seek(0x1820);
            ProjectNames[project.get<std::string>()] = bnr.readString(0x20);
            bnr.seek(0x20);
            ImageFormat::Decode::RGB5A3(&bnr, 96, 32, bnrImgData.data());
        }

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 96, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, bnrImgData.data());
		glBindTexture(GL_TEXTURE_2D, 0);
        LGenUtility::Log << "[Project Manager] Loaded Banner Image for Project " << project.get<std::string>() << std::endl;
    }

	if(!std::filesystem::exists(std::filesystem::current_path() / "roots")){
        LGenUtility::Log << "[Project Manager] Created Roots Directory" << std::endl;
		std::filesystem::create_directory(std::filesystem::current_path() / "roots");
	}
}

void Render(){
    std::cout << "[Project Manager] Rendering Project Manager" << std::endl;
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	// Project Manager
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x * 0.65f, ImGui::GetMainViewport()->Size.y * 0.75f}, ImGuiCond_Always);

	if (ImGui::BeginPopupModal("ProjectManager", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::Text("Game Roots");
		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::BeginChild("##projectsPanel");
            if(ShowNewProjectDialog) {
                ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - (ImGui::GetContentRegionAvail().x * 0.65f)) * 0.65f);
                ImGui::SetCursorPosY((ImGui::GetContentRegionAvail().y - (ImGui::GetContentRegionAvail().y * 0.65f)) * 0.65f);
                ImGui::BeginChild("##newProject", ImVec2(ImGui::GetContentRegionAvail().x * 0.65f, ImGui::GetContentRegionAvail().y * 0.65f), ImGuiChildFlags_Border);

                ImGui::EndChild();
            } else {
                std::size_t toDelete = -1;
                for(std::size_t id = 0; id < ProjectsJson["projects"].size(); id++){
                    auto project = ProjectsJson["projects"][id];
                    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - (ImGui::GetContentRegionAvail().x * 0.90f)) * 0.65f);
                    ImGui::BeginChild(std::format("{}##{}", project.get<std::string>(), id).c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.90f, 82.0f), ImGuiChildFlags_Border);
                        if(ImGui::IsWindowHovered()){
                            float ypos = ImGui::GetCursorPosY();
                            ImGui::SetCursorPosY(ypos + 24);
                            ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 10);
                            ImGui::Text(ICON_FK_TRASH);
                            if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
                                toDelete = id;
                            } else if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){

                                OPTIONS.mRootPath = project.get<std::string>();
                                GCResourceManager.Init();
                                LEditorScene::GetEditorScene()->LoadResFromRoot();

                                LResUtility::LoadMapThumbnails(project.get<std::string>());
                                ImGui::CloseCurrentPopup();

                                DOL dol;
                                dol.LoadDOLFile(std::filesystem::path(OPTIONS.mRootPath) / "sys" / "main.dol");

                                JustClosed = true;
                            }
                            ImGui::SetCursorPosY(ypos);
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
                        }
                        ImGui::Image(static_cast<uintptr_t>(ProjectBanners[project.get<std::string>()]), {192, 64});
                        ImGui::SameLine();
                        ImGui::BeginGroup();
                        ImGui::Text(ProjectNames[project.get<std::string>()].c_str());
                        ImGui::Text(project.get<std::string>().c_str());
                        ImGui::EndGroup();
                    ImGui::EndChild();                    
                }

                if(toDelete != -1){
                    std::filesystem::remove_all(ProjectsJson["projects"][toDelete].get<std::string>());
                    ProjectsJson["projects"].erase(toDelete);
                    std::ofstream{std::filesystem::current_path() / "res" / "projects.json"} << ProjectsJson;
                    Init(); // reinit
                }

                ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - (ImGui::GetContentRegionAvail().x * 0.90f)) * 0.65f);
                ImGui::BeginChild("##newProject", ImVec2(ImGui::GetContentRegionAvail().x * 0.90f, 38.0f), ImGuiChildFlags_Border);
                    if(ImGui::IsWindowHovered()){
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

                        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                            ShowNewProjectDialog = true;
                        }
                    }
                    ImGui::Text(ICON_FK_PLUS " New");
                ImGui::EndChild();
            }
		ImGui::EndChild();
        if(ShowNewProjectDialog){
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
	}

    if(ShowNewProjectDialog){
        ImGui::OpenPopup("NewRootNameDiag");
        ShowNewProjectDialog = false;
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if(ImGui::BeginPopupModal("NewRootNameDiag", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar)){
        ImGui::Text("Root Name");
        LUIUtility::RenderTooltip("Name of the folder booldozer will create, can't conflict with other root dir names!");
        ImGui::Separator();
        LUIUtility::RenderTextInput("##projectRootName", &NewProjectRootName);
        if(ImGui::Button("Next")){
            ImGuiFileDialog::Instance()->OpenModal("newRootDialog", "Create Project Root", "GameCube Disk Image (*.gcm *.iso){.gcm,.iso}", std::filesystem::current_path().string());
            OpenNewRoot = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")){
            NewProjectRootName = "";
            ImGui::CloseCurrentPopup();
            ImGui::OpenPopup("ProjectManager");
        }
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if(LUIUtility::RenderFileDialog("newRootDialog", NewProjectRootPath)){
        std::shared_ptr<Disk::Image> image = Disk::Image::Create();
        bStream::CFileStream imageStream(NewProjectRootPath, bStream::Endianess::Big, bStream::OpenMode::In);
        if(!image->Load(&imageStream)){
            ImGui::OpenPopup("ImgOpenError");
        } else {
            image->GetRoot()->SetName(NewProjectRootName);

            if(!std::filesystem::exists(std::filesystem::current_path() / "roots" / image->GetRoot()->GetName())){
                std::filesystem::path curpath = std::filesystem::current_path();
                std::filesystem::current_path(std::filesystem::current_path() / "roots");
                
                ExtractFolderISO(image->GetRoot());

                std::filesystem::current_path(curpath);
            }

            ProjectsJson["projects"].push_back(nlohmann::json(std::filesystem::current_path() / "roots" / NewProjectRootName));
            std::ofstream{std::filesystem::current_path() / "res" / "projects.json"} << ProjectsJson;
            Init(); // reinit
        }
        ImGui::OpenPopup("ProjectManager");
    } else if(!ImGuiFileDialog::Instance()->IsOpened() && OpenNewRoot) {
        ImGui::OpenPopup("ProjectManager");
        OpenNewRoot = false;
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if(ImGui::BeginPopupModal("ImgOpenError", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar)){
        ImGui::Text("Error");
        ImGui::Separator();
        ImGui::Text("Couldn't Extract Selected Image!");
        if(ImGui::Button("Ok")){
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

}