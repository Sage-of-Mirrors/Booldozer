#include "ui/BooldozerEditor.hpp"
#include <io/JmpIO.hpp>
#include <../lib/bStream/bstream.h>
#include <iostream>
#include <vector>

#include <thread>
#include <mutex>

#include "ResUtil.hpp"
#include "UIUtil.hpp"
#include "DOL.hpp"
#include "DOM/FurnitureDOMNode.hpp"
#include "io/StaticMapDataIO.hpp"
#include "Options.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "misc/cpp/imgui_stdlib.h"
#include <glad/glad.h>

#include <bzlib.h>
#include "../lib/bsdifflib/bspatchlib.h"
#include "../lib/bsdifflib/bsdifflib.h"

#include <cstdlib>

#include "DOM/CameraAnimationDOMNode.hpp"
#include "scene/ModelViewer.hpp"
#include <format>

#include "stb_image.h"
#include "stb_image_write.h"
#include "GCM.hpp"
#include "picosha2.h"
#include "ProjectManager.hpp"
#include "ui/BloEditor.hpp"

namespace {
	char* patchErrorMsg { nullptr };
	std::thread mapOperationThread {};
	std::mutex loadLock {};
	bool mapLoading { false };
	uint32_t bannerEditorTexPreview { 0 };
}

LBooldozerEditor::LBooldozerEditor()
{
	CurrentMode = EEditorMode::Actor_Mode;
	mCurrentMode = &mActorMode;

	LResUtility::LoadUserSettings();
}

LBooldozerEditor::~LBooldozerEditor(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glDeleteFramebuffers(1, &mFbo);
	glDeleteRenderbuffers(1, &mRbo);
	glDeleteTextures(1, &mViewTex);
	glDeleteTextures(1, &mPickTex);

	CameraAnimation::CleanupPreview();
	PreviewWidget::CleanupPreview();

	LResUtility::CleanupThumbnails();
}

void PackFolderISO(std::shared_ptr<Disk::Image> img, std::shared_ptr<Disk::Folder> folder, std::filesystem::path path){
    std::filesystem::current_path(path);

    for (auto const& dir_entry : std::filesystem::directory_iterator(path)){
        if(std::filesystem::is_directory(dir_entry.path())){
            std::shared_ptr<Disk::Folder> subdir = Disk::Folder::Create(img);
            subdir->SetName(dir_entry.path().filename().string());
            folder->AddSubdirectory(subdir);

            PackFolderISO(img, subdir, dir_entry.path());

        } else {
            std::shared_ptr<Disk::File> file = Disk::File::Create();

            bStream::CFileStream fileStream(dir_entry.path().string(), bStream::Endianess::Big, bStream::OpenMode::In);

            uint8_t* fileData = new uint8_t[fileStream.getSize()];
            fileStream.readBytesTo(fileData, fileStream.getSize());

            file->SetData(fileData, fileStream.getSize());
            file->SetName(dir_entry.path().filename().string());

            folder->AddFile(file);

            delete[] fileData;
        }
    }
    std::filesystem::current_path(std::filesystem::current_path().parent_path());
}

void PackISO(std::filesystem::path path){
    std::shared_ptr<Disk::Image> image = Disk::Image::Create();
    std::shared_ptr<Disk::Folder> root = Disk::Folder::Create(image);

    image->SetRoot(root);
    root->SetName(path.filename().string());

    PackFolderISO(image, root, OPTIONS.mRootPath);

    // check that we have the right files in sys
    if(root->GetFolder("sys") == nullptr){
        std::cout << "Root missing sys folder" << std::endl;
        return;
    }

    if(root->GetFile("sys/apploader.img") == nullptr){
        std::cout << "Root missing sys/apploader.img" << std::endl;
        return;
    }

    if(root->GetFile("sys/boot.bin") == nullptr){
        std::cout << "Root missing sys/boot.bin" << std::endl;
        return;
    }

    if(root->GetFile("sys/bi2.bin") == nullptr){
        std::cout << "Root missing sys/bi2.bin" << std::endl;
        return;
    }

    if(root->GetFile("sys/main.dol") == nullptr){
        std::cout << "Root missing sys/main.dol" << std::endl;
        return;
    }

	std::filesystem::path writeGCM = path.replace_extension(".gcm");

    image->SaveToFile(writeGCM.string());

	loadLock.lock();
	mapLoading = false;
	loadLock.unlock();

}

void LBooldozerEditor::LoadMap(std::string path, LEditorScene* scene){
	OpenMap(path);

	OPTIONS.mLastOpenedMap = path;
	LResUtility::SaveUserSettings();

	loadLock.lock();
	mapLoading = false;
	loadLock.unlock();
}

void LBooldozerEditor::SaveMap(std::string path){
	SaveMapToArchive(path);

	OPTIONS.mLastSavedDirectory = path;
	LResUtility::SaveUserSettings();

	loadLock.lock();
	mapLoading = false;
	loadLock.unlock();
}

void LBooldozerEditor::SaveActorConfigs(){

	mGhostConfigs.SaveConfigsToFile();

	loadLock.lock();
	mapLoading = false;
	loadLock.unlock();
}

void LBooldozerEditor::Render(float dt, LEditorScene* renderer_scene)
{
	std::string path = "";

	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_AutoHideTabBar;
	mMainDockSpaceID = ImGui::DockSpaceOverViewport(0, mainViewport, dockFlags);

	if(!bInitialized){

		glGenFramebuffers(1, &mFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

		glGenRenderbuffers(1, &mRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);

		glGenTextures(1, &mViewTex);
		glGenTextures(1, &mPickTex);

		glBindTexture(GL_TEXTURE_2D, mViewTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, mPickTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, 1280, 720, 0, GL_RED_INTEGER, GL_INT, nullptr);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mViewTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPickTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		CameraAnimation::InitPreview();
		PreviewWidget::InitPreview();

		ImGui::DockBuilderRemoveNode(mMainDockSpaceID); // clear any previous layout
		ImGui::DockBuilderAddNode(mMainDockSpaceID, dockFlags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(mMainDockSpaceID, mainViewport->Size);


		mDockNodeLeftID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Left, 0.27f, nullptr, &mMainDockSpaceID);
		mDockNodeRightID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Right, 0.25f, nullptr, &mMainDockSpaceID);
		mDockNodeDownLeftID = ImGui::DockBuilderSplitNode(mDockNodeLeftID, ImGuiDir_Down, 0.5f, nullptr, &mDockNodeUpLeftID);

		ImGui::DockBuilderDockWindow("sceneHierarchy", mDockNodeUpLeftID);
		ImGui::DockBuilderDockWindow("detailWindow", mDockNodeDownLeftID);
		ImGui::DockBuilderDockWindow("toolWindow", mDockNodeRightID);
		ImGui::DockBuilderDockWindow("viewportWindow", mMainDockSpaceID);

		ImGui::DockBuilderFinish(mMainDockSpaceID);
		bInitialized = true;

		ProjectManager::Init();
		ImGui::OpenPopup("ProjectManager");
	}

	ProjectManager::Render();

	BloEditor::Render();

	if(mOpenProjectManager){
		ImGui::OpenPopup("ProjectManager");
		mOpenProjectManager = false;
		GetSelectionManager()->ClearSelection();
		renderer_scene->Clear();
		mLoadedMap = nullptr;
	}

	if (ImGui::BeginPopupModal("Map Extraction Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::NewLine();
		ImGui::Text("Couldn't rip static map data from dol. Did you edit your DOL already?");
		ImGui::NewLine();
		ImGui::Text("How to fix:");
		if(ImGui::TreeNode("I want my edited rooms and doors to show up in game!")){
			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::Text("Backup edited dol");
			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::Text("Extract *CLEAN DOL* from iso/gcm");
			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::Text("Place clean dol at '<root>/sys/main.dol'");
			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::Text("Restart Booldozer and apply patch");
			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::Text("Re-apply your modifications to newly patched dol");
			ImGui::TreePop();
		}

		if(ImGui::TreeNode("Just show me the map!")){
			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::Text("Extract *CLEAN DOL* from iso/gcm");
			ImGui::Bullet();
			ImGui::SameLine();
			ImGui::Text("Place clean dol at '<root>/sys/.main_dol_backup'");
			ImGui::TreePop();
		}

		ImGui::NewLine();
		ImGui::Separator();

		if (ImGui::Button("Ok")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// When you open a project, check if it's dol needs to be patched by seeing if the backup exists or not.
	// This hash is done a few times in a few places, when opening maps for the first time specifically since it needs to extract map data
	if(ProjectManager::JustClosed){
		if(!std::filesystem::exists(std::filesystem::path(OPTIONS.mRootPath) / "sys" / ".main_dol_backup")){
			std::ifstream f(std::filesystem::path(OPTIONS.mRootPath) / "sys" / "main.dol", std::ios::binary);
			std::vector<unsigned char> s(picosha2::k_digest_size);
			picosha2::hash256(f, s.begin(), s.end());

			std::string chksum = picosha2::hash256_hex_string(s);

			LGenUtility::Log << "[ProjectManager]: SHA256 of executable is " << chksum << std::endl;
			if(chksum == "4e233ab2509e055894dbfef9cf4c5c07668b312ee2c2c44362b7d952308ce95a"){
				LGenUtility::Log << "[DOL]: Executable is clean" << std::endl;
				ImGui::OpenPopup("Unpatched DOL");
				OPTIONS.mIsDOLPatched = false;
			} else {
				OPTIONS.mIsDOLPatched = true;
			}
		}

		ProjectManager::JustClosed = false;
	}

	if (ImGui::BeginPopupModal("Unpatched DOL", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("This root has a clean DOL. Certain edits will not be reflected in game!");
		ImGui::Text("Apply externalized map data patch?");
		ImGui::Separator();
		if (ImGui::Button("Yes")) {
			std::filesystem::path patchPath = RES_BASE_PATH / "externalizemaps.patch";
			std::filesystem::path dolPath = std::filesystem::path(OPTIONS.mRootPath) / "sys" / "main.dol";
			std::filesystem::path patchedPath = std::filesystem::path(OPTIONS.mRootPath) / "sys" / "patched.dol";
			std::filesystem::path backupPath = std::filesystem::path(OPTIONS.mRootPath) / "sys" / ".main_dol_backup";

			if(std::filesystem::exists(patchedPath)){
				std::filesystem::remove(patchedPath);
			}

			if(std::filesystem::exists(backupPath)){
				std::filesystem::remove(backupPath);
			}

			//Copy DOL to a backup
			std::filesystem::copy(dolPath, std::filesystem::path(OPTIONS.mRootPath) / "sys" / ".main_dol_backup");

			if(std::filesystem::exists(patchPath)){
				patchErrorMsg = bspatch(dolPath.string().c_str(), patchedPath.string().c_str(), patchPath.string().c_str());
				if(patchErrorMsg == NULL){
					std::filesystem::remove(dolPath);
					std::filesystem::copy(patchedPath, dolPath);
					std::filesystem::remove(patchedPath);
					OPTIONS.mIsDOLPatched = true;
				} else {
					ImGui::CloseCurrentPopup();
					ImGui::OpenPopup("Patching Error");
				}
			} else {
				ImGui::OpenPopup("Missing Patch File");
				ImGui::CloseCurrentPopup();
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Patching Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Error Applying Patch: %s", patchErrorMsg);
		ImGui::Separator();
		if (ImGui::Button("Ok")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Missing Patch File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Patch file not found!");
		ImGui::Separator();
		if (ImGui::Button("Ok")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Loading Map", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		ImGui::AlignTextToFramePadding();
		ImGui::Spinner("##loadmapSpinner", 5.0f, 2, col);
		ImGui::SameLine();
		ImGui::Text("Loading Map...");

		loadLock.lock();
		if(mapLoading == false){
			LGenUtility::Log << "[BooldozerEditor]: Joining load/append thread" << std::endl;
			mapOperationThread.join();

			auto rooms = mLoadedMap->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);

			if(rooms.size() > 0){
				renderer_scene->SetRoom(rooms[0]);
			}

			ImGui::CloseCurrentPopup();
		}
		loadLock.unlock();

		ImGui::EndPopup();
	}

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Saving Map", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		ImGui::AlignTextToFramePadding();
		ImGui::Spinner("##loadmapSpinner", 5.0f, 2, col);
		ImGui::SameLine();
		ImGui::Text("Saving Map...");

		loadLock.lock();
		if(mapLoading == false){
			LGenUtility::Log << "[BooldozerEditor]: Joining save thread" << std::endl;
			mapOperationThread.join();
			ImGui::CloseCurrentPopup();
		}
		loadLock.unlock();

		ImGui::EndPopup();
	}

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("SavingConfigsModal", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		ImGui::AlignTextToFramePadding();
		ImGui::Spinner("##loadmapSpinner", 5.0f, 2, col);
		ImGui::SameLine();
		ImGui::Text("Saving Actor Configurations...");

		loadLock.lock();
		if(mapLoading == false){
			LGenUtility::Log << "[BooldozerEditor]: Joining save thread" << std::endl;
			mapOperationThread.join();
			ImGui::CloseCurrentPopup();
		}
		loadLock.unlock();

		ImGui::EndPopup();
	}


    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("ControlsDialog", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Controls");

		ImGui::BulletText("Change Gizmo");
		ImGui::Spacing();
		ImGui::SameLine();
		ImGui::BulletText("1: Translation");

		ImGui::Spacing();
		ImGui::SameLine();
		ImGui::BulletText("2: Rotation");

		ImGui::Spacing();
		ImGui::SameLine();
		ImGui::BulletText("3: Scale");

		if(ImGui::Button("Close")){
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("ExportGCMPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		ImGui::AlignTextToFramePadding();
		ImGui::Spinner("##loadmapSpinner", 5.0f, 2, col);
		ImGui::SameLine();
		ImGui::Text("Exporting GCM...");

		loadLock.lock();
		if(mapLoading == false){
			LGenUtility::Log << "[BooldozerEditor]: Joining export gcm thread" << std::endl;
			mapOperationThread.join();

			ImGui::CloseCurrentPopup();
		}
		loadLock.unlock();

		ImGui::EndPopup();
	}


	if(mOpenControlsDialog){
		ImGui::OpenPopup("ControlsDialog");
		mOpenControlsDialog = false;
	}

	if(mClickedMapSelect){
		ImGui::OpenPopup("Map Select");
		mClickedMapSelect = false;
	}

	if(mClickedMapClear){
		ImGui::OpenPopup("Clear Map");
		mClickedMapClear = false;
	}

	if(mOpenActorEditor){
		ImGui::OpenPopup("ActorEditor");
		PreviewWidget::SetActive();
		mOpenActorEditor = false;
	}

	if(mOpenBannerEditor){

		glCreateTextures(GL_TEXTURE_2D, 1, &bannerEditorTexPreview);
		glTextureStorage2D(bannerEditorTexPreview, 1, GL_RGBA8, 96, 32);
		glTextureSubImage2D(bannerEditorTexPreview, 0, 0, 0, 96, 32, GL_RGBA, GL_UNSIGNED_BYTE, GCResourceManager.mBannerImage);

		ImGui::OpenPopup("BannerEditor");
		mOpenBannerEditor = false;
	}

	if(mOpenMenuEditor){
		ImGui::OpenPopup("BloEditorTool");
		mOpenMenuEditor = false;
	}

	if(mSaveConfigsClicked){
		loadLock.lock();
		mapLoading = true;
		loadLock.unlock();

		mapOperationThread = std::thread(&LBooldozerEditor::SaveActorConfigs, std::ref(*this));
		ImGui::OpenPopup("SavingConfigsModal");
	}

	if(mSaveMapClicked){
		loadLock.lock();
		mapLoading = true;
		loadLock.unlock();

		mapOperationThread = std::thread(&LBooldozerEditor::SaveMap, std::ref(*this), (std::filesystem::path(OPTIONS.mRootPath) / "files" / "Map" / std::format("map{}.szp", mSelectedMap)).string());
		ImGui::OpenPopup("Saving Map");
		// This gets set later after we save the thumbnail SaveMapClicked = false;
	}

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x * 0.32f, ImGui::GetMainViewport()->Size.y * 0.8f}, ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Map Select", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		auto mapNames = LResUtility::GetNameMap("MapNames");
		bool openedMap = false;
		bool hovered = false;
		bool closeEdit = false;
		for(int x = 0; x <= 13; x++){
			ImGui::BeginChild(std::format("##map{}Select", x).data(), ImVec2(ImGui::GetContentRegionAvail().x, 80.0f), ImGuiChildFlags_Border);
				if(ImGui::IsWindowHovered()){
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
					hovered = true;
				}
				// TODO: thumbnails - from starforge
				uint32_t imgId = LResUtility::GetMapThumbnail(x);
				ImGui::Image((ImTextureID)imgId, ImVec2(84, 64), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
				ImGui::SameLine();
				ImGui::BeginGroup();
				std::string name = mapNames["names"][x].get<std::string>();
				if(x != mMapNameDialogEditingNameIdx){
					ImGui::Text(name.data()); // map config? so they can be named?
					ImGui::SameLine();
					ImGui::Text(ICON_FK_PENCIL);
					if(ImGui::IsItemClicked()){
						mMapNameDialogEditingNameIdx = x;
						mMapNameDialogEditingNameStr = name;
					}
				} else {
					ImGui::InputText("##mapName", &mMapNameDialogEditingNameStr);
					if(ImGui::IsKeyDown(ImGuiKey_Enter)){
						mapNames["names"][x] = nlohmann::json(mMapNameDialogEditingNameStr);
						LResUtility::SetNameMap("MapNames", mapNames);
						std::ofstream namesConfig((RES_BASE_PATH / "names" / "MapNames.json").string());
						namesConfig << mapNames;
						mMapNameDialogEditingNameIdx = -1;
						mMapNameDialogEditingNameStr = "";
					}
					ImGui::SameLine();
					ImGui::Text(ICON_FK_CHECK);
					if (ImGui::IsItemClicked()){
						closeEdit = true;
					}

				}

				if(mMapNameDialogEditingNameIdx == -1 && (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false))){
					ImGui::CloseCurrentPopup();
					GetSelectionManager()->ClearSelection();
					renderer_scene->Clear();

					openedMap = true;
					mSelectedMap = x;
				}

				ImGui::EndGroup();
			ImGui::EndChild();
			if(openedMap) break;
		}
		if(closeEdit){
			mapNames["names"][mMapNameDialogEditingNameIdx] = nlohmann::json(mMapNameDialogEditingNameStr);
			LResUtility::SetNameMap("MapNames", mapNames);
			std::ofstream namesConfig((RES_BASE_PATH / "names" / "MapNames.json").string());
			namesConfig << mapNames;
			mMapNameDialogEditingNameIdx = -1;
			mMapNameDialogEditingNameStr = "";
		}
		ImGui::EndPopup();

		if(openedMap){
			loadLock.lock();
			mapLoading = true;
			loadLock.unlock();
			mapOperationThread = std::thread(&LBooldozerEditor::LoadMap, std::ref(*this), (std::filesystem::path(OPTIONS.mRootPath) / "files" / "Map" / std::format("map{}.szp", mSelectedMap)).string(), renderer_scene);
			ImGui::OpenPopup("Loading Map");
		}
	}

    center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Clear Map", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("You are about to clear *everything* from this map, including all room archives, are you sure?");

		ImGui::Separator();
		if (ImGui::Button("Yes")) {
			renderer_scene->Clear();
			GetSelectionManager()->ClearSelection();

			auto rooms = mLoadedMap->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);

			std::shared_ptr<LRoomDOMNode> newRoom = std::make_shared<LRoomDOMNode>("Room 0");
			std::shared_ptr<LRoomDataDOMNode> newRoomData = std::make_shared<LRoomDataDOMNode>("Room 0");

			std::string resourcePathRoot = std::filesystem::path(rooms[0]->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0]->GetResourcePath()).parent_path().string();
			newRoomData->SetRoomResourcePath(std::format("{}/room_00.arc", resourcePathRoot));

			std::string resPathInRoot = std::format("{}/{}{}", OPTIONS.mRootPath, "files", newRoomData->GetResourcePath());

			for(auto ent : mLoadedMap->GetChildrenOfType<LEntityDOMNode>(EDOMNodeType::Entity)){
				mLoadedMap->RemoveChild(ent);
			}

			for(auto mirror : mLoadedMap->GetChildrenOfType<LMirrorDOMNode>(EDOMNodeType::Mirror)){
				mLoadedMap->RemoveChild(mirror);
			}

			for(auto door : mLoadedMap->GetChildrenOfType<LDoorDOMNode>(EDOMNodeType::Door)){
				mLoadedMap->RemoveChild(door);
			}

			for(auto room : rooms){
				mLoadedMap->RemoveChild(room);
			}

			// Clear all room resources
			std::filesystem::remove_all(std::filesystem::path(resPathInRoot).parent_path());

			if(!std::filesystem::exists(std::filesystem::path(resPathInRoot).parent_path())){
				std::filesystem::create_directories(std::filesystem::path(resPathInRoot).parent_path());
			}

			LGenUtility::Log << "[Editor]: Room resource path " << resPathInRoot << std::endl;
			if(!std::filesystem::exists(resPathInRoot)){
				std::shared_ptr<Archive::Rarc> arc = Archive::Rarc::Create();
				std::shared_ptr<Archive::Folder> root = Archive::Folder::Create(arc);
				arc->SetRoot(root);
				// now before save, construct as path to replace directory seperators with proper system seps
				arc->SaveToFile(std::filesystem::path(resPathInRoot).string());
			}

			// Setup default room
			newRoomData->SetRoomID(0);
			newRoomData->SetRoomIndex(0);

			newRoomData->SetMin({-1000, 0, -1000});
			newRoomData->SetMax({1000, 1000, 1000});
			newRoom->AddChild(newRoomData);
			newRoom->SetRoomNumber(0);
			newRoomData->GetAdjacencyList().push_back(newRoom);

			mLoadedMap->AddChild(newRoom);

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("No")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("BannerEditor", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiChildFlags_AlwaysAutoResize))
	{
		ImGui::Text("Banner");
		ImGui::Separator();

		ImGui::Image(static_cast<uintptr_t>(bannerEditorTexPreview), {96, 32});
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::Text(ICON_FK_FOLDER_OPEN);
		if(ImGui::IsItemClicked()){
			ImGuiFileDialog::Instance()->OpenModal("openNewBanner", "Open Banner Image", "Image (*.png){.png}", ".");
			ImGui::CloseCurrentPopup();
		}
		ImGui::Text(ICON_FK_FLOPPY_O);
		if(ImGui::IsItemClicked()){
			ImGuiFileDialog::Instance()->OpenModal("saveBannerImage", "Save Banner Image Titlecard", "Image (*.png){.png}", ".");
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndGroup();
		ImGui::BeginGroup();
		ImGui::InputText("Short Title", GCResourceManager.mBanner.mGameTitle, sizeof(GCResourceManager.mBanner.mGameTitle));
		ImGui::InputText("Short Developer", GCResourceManager.mBanner.mDeveloper, sizeof(GCResourceManager.mBanner.mDeveloper));
		ImGui::InputText("Title", GCResourceManager.mBanner.mGameTitleFull, sizeof(GCResourceManager.mBanner.mGameTitleFull));
		ImGui::InputText("Developer", GCResourceManager.mBanner.mDeveloperFull, sizeof(GCResourceManager.mBanner.mDeveloperFull));
		ImGui::InputTextMultiline("Description", GCResourceManager.mBanner.mGameDescription, sizeof(GCResourceManager.mBanner.mGameDescription));
		ImGui::EndGroup();

		if(ImGui::Button("Save")){
			bStream::CMemoryStream bnrImgStream(GCResourceManager.mBanner.mImage, sizeof(GCResourceManager.mBanner.mImage), bStream::Endianess::Big, bStream::OpenMode::Out);

			std::vector<uint8_t> bnrImgData(96*32*4);
			std::memcpy(bnrImgData.data(), GCResourceManager.mBannerImage, sizeof(GCResourceManager.mBannerImage));

			ImageFormat::Encode::RGB5A3(&bnrImgStream, 96, 32, bnrImgData.data());

			std::filesystem::path bannerPath(std::filesystem::path(OPTIONS.mRootPath) / "files" / "opening.bnr");
			bStream::CFileStream bnr(bannerPath.string(), bStream::Endianess::Big, bStream::OpenMode::Out);
			bnr.writeBytes((uint8_t*)&GCResourceManager.mBanner, sizeof(GCResourceManager.mBanner));

			ImGui::CloseCurrentPopup();
			glDeleteTextures(1, &bannerEditorTexPreview);
		}

		ImGui::SameLine();
		if (ImGui::Button("No")) {
			ImGui::CloseCurrentPopup();
			glDeleteTextures(1, &bannerEditorTexPreview);
		}

		ImGui::EndPopup();
	}

	std::string imgPath;
	if(LUIUtility::RenderFileDialog("openNewBanner", imgPath)){
		int x,y,n;
		unsigned char* img = stbi_load(imgPath.c_str(), &x, &y, &n, 0);

		LGenUtility::Log << n << std::endl;

		if(x == 96 && y == 32 && n == 4){
			std::memcpy(GCResourceManager.mBannerImage, img, 96*32*4);
		}

		glTextureSubImage2D(bannerEditorTexPreview, 0, 0, 0, 96, 32, GL_RGBA, GL_UNSIGNED_BYTE, img);

		delete img;

		ImGui::OpenPopup("BannerEditor");
	}

	if(LUIUtility::RenderFileDialog("saveBannerImage", imgPath)){
		stbi_write_png(imgPath.c_str(), 96, 32, 4, GCResourceManager.mBannerImage, 96*4);
		ImGui::OpenPopup("BannerEditor");
	}

	if (LUIUtility::RenderFileDialog("appendMapDlg", path))
	{
		GetSelectionManager()->ClearSelection();

		loadLock.lock();
		mapLoading = true;
		loadLock.unlock();

		mapOperationThread = std::thread(&LBooldozerEditor::AppendMap, std::ref(*this), path);
		ImGui::OpenPopup("Loading Map");
	}

	if (LUIUtility::RenderFileDialog("exportGCMDlg", path)){
		loadLock.lock();
		mapLoading = true;
		loadLock.unlock();

		mapOperationThread = std::thread(&PackISO, path);
		ImGui::OpenPopup("ExportGCMPopup");
	}

	if (mapLoading == false && mLoadedMap != nullptr && !mLoadedMap->Children.empty() && mCurrentMode != nullptr){
		EEditorMode mode;
		mCurrentMode->Render(mLoadedMap, renderer_scene, mode);
		if(mode != CurrentMode){
			mCurrentMode->GetSelectionManager()->ClearSelection();
			CurrentMode = mode;
			ChangeMode();
		}
	}

	mSaveConfigsClicked = mGhostConfigs.RenderUI();

	CameraAnimation::RenderPreview();
	PreviewWidget::RenderPreview();

	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::Begin("viewportWindow");

	//TODO: check if window size changes so texture can be resized
	ImVec2 winSize = ImGui::GetContentRegionAvail();
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();


	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mRbo);

	if(mSaveMapClicked){
		LResUtility::SaveMapThumbnail(static_cast<uint32_t>(winSize.x), static_cast<uint32_t>(winSize.y), mSelectedMap);
		// Reload Thumbnails
		LResUtility::CleanupThumbnails();
		LResUtility::LoadMapThumbnails();

		mSaveMapClicked = false;
	}

	if(winSize.x != mPrevWinWidth || winSize.y != mPrevWinHeight){
		glDeleteTextures(1, &mViewTex);
		glDeleteTextures(1, &mPickTex);
		glDeleteRenderbuffers(1, &mRbo);

		glGenRenderbuffers(1, &mRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (uint32_t)winSize.x, (uint32_t)winSize.y);

		glGenTextures(1, &mViewTex);
		glGenTextures(1, &mPickTex);

		glBindTexture(GL_TEXTURE_2D, mViewTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, mPickTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RED_INTEGER, GL_INT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mViewTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPickTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

		GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		//assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	glViewport(0, 0, (uint32_t)winSize.x, (uint32_t)winSize.y);

	mPrevWinWidth = winSize.x;
	mPrevWinHeight = winSize.y;

	glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int32_t unused = -1;

	glClearTexImage(mPickTex, 0, GL_RED_INTEGER, GL_INT, &unused);

	// is this slow? shouldnt be but is it???
	loadLock.lock();
	if(!mapLoading) renderer_scene->RenderSubmit((uint32_t)winSize.x,  (uint32_t)winSize.y);
	loadLock.unlock();


	ImGui::Image(static_cast<uintptr_t>(mViewTex), winSize, {0.0f, 1.0f}, {1.0f, 0.0f});

	if(ImGui::IsItemHovered() && (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right))){
		ImGui::SetWindowFocus(nullptr);
		renderer_scene->SetActive(true);
	} else {
		renderer_scene->SetActive(false);
	}

	if(!ImGui::GetIO().WantTextInput){
		if(ImGui::IsKeyPressed(ImGuiKey_1)){
			mCurrentMode->mGizmoMode = ImGuizmo::OPERATION::TRANSLATE;
		} else if (ImGui::IsKeyPressed(ImGuiKey_2)){
			mCurrentMode->mGizmoMode = ImGuizmo::OPERATION::ROTATE;
		} else if (ImGui::IsKeyPressed(ImGuiKey_3)){
			mCurrentMode->mGizmoMode = ImGuizmo::OPERATION::SCALE;
		}
	}

	if(ImGui::IsKeyPressed(ImGuiKey_Escape)){
		GetSelectionManager()->ClearSelection();
	}

	if(ImGui::IsItemClicked() && mLoadedMap != nullptr && !ImGuizmo::IsOver()){
		int32_t id;
		ImVec2 mousePos = ImGui::GetMousePos();

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glReadPixels(mousePos.x - cursorPos.x, ((uint32_t)winSize.y) - (mousePos.y - cursorPos.y), 1, 1, GL_RED_INTEGER, GL_INT, (void*)&id);

		if(id == -1){
			GetSelectionManager()->ClearSelection();
		} else {
			for(auto node : mLoadedMap->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
				if(mCurrentMode == &mEventMode){
					if(node->GetID() == id && node->GetNodeType() == EDOMNodeType::Event){
						auto eventDataNodes = mLoadedMap->GetChildrenOfType<LEventDataDOMNode>(EDOMNodeType::EventData);
						for (auto datanode : eventDataNodes){
							if(datanode->GetEventNo() == std::static_pointer_cast<LEventDOMNode>(node)->GetEventNo()){
								GetSelectionManager()->AddToSelection(datanode);
								break;
							}
						}
					}
				} else if(mCurrentMode == &mPathMode){
					if(node->GetID() == id && (node->GetNodeType() == EDOMNodeType::Path || node->GetNodeType() == EDOMNodeType::PathPoint)){
						GetSelectionManager()->AddToSelection(node);
					}
				} else if(mCurrentMode == &mDoorMode){
					if(node->GetID() == id && node->GetNodeType() == EDOMNodeType::Door){
						GetSelectionManager()->AddToSelection(node);
					}
				} else if(mCurrentMode == &mEnemyMode){
					if(node->GetID() == id && node->GetNodeType() == EDOMNodeType::Enemy){
						GetSelectionManager()->AddToSelection(node);
					}
				} else if(mCurrentMode == &mActorMode) {
					if(node->GetID() == id){
						GetSelectionManager()->AddToSelection(node);
					}
				}
			}
		}
	}

	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, winSize.x, winSize.y);

	if(mCurrentMode != nullptr) mCurrentMode->RenderGizmo(renderer_scene);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	ImGui::End();


	// Popups
	RenderNoRootPopup();



	//fbo copy
}

void LBooldozerEditor::OpenMap(std::string file_path)
{

	mLoadedMap = std::make_shared<LMapDOMNode>();
	LGenUtility::Log << "[MapDOMNode] Starting Load Map" << std::endl;
	mLoadedMap->LoadMap(std::filesystem::path(file_path));

	mGhostConfigs.LoadConfigs(mLoadedMap);
}

void LBooldozerEditor::AppendMap(std::string map_path)
{
	auto appendMap = std::make_shared<LMapDOMNode>();
	if(appendMap->LoadMap(std::filesystem::path(map_path))){
		mLoadedMap->AppendMap(appendMap);
	}
	loadLock.lock();
	mapLoading = false;
	loadLock.unlock();
}

void LBooldozerEditor::SaveMapToArchive(std::string file_path)
{
	mLoadedMap->SaveMapToArchive(file_path);
}

void LBooldozerEditor::onOpenMapCB()
{
	mClickedMapSelect = true;
}

void LBooldozerEditor::onAppendMapCB()
{
	ImGuiFileDialog::Instance()->OpenDialog("appendMapDlg", "Append Map Archive to Current Map", "Archives (*.arc *.szs *.szp){.arc,.szs,.szp}", OPTIONS.mLastOpenedMap);
}

void LBooldozerEditor::onClearMapCB()
{
	mClickedMapClear = true;
}

void LBooldozerEditor::onSaveMapArchiveCB()
{
	if(mSelectedMap != -1 && mLoadedMap != nullptr){
		mSaveMapClicked = true;
	}
}

void LBooldozerEditor::onGCMExportCB()
{
	ImGuiFileDialog::Instance()->OpenDialog("exportGCMDlg", "Export GCM", "GameCube Disk Image (*.gcm *.iso *.szp){.iso,.gcm}", OPTIONS.mLastOpenedMap);
}

void LBooldozerEditor::onPlaytestCB()
{
	// This seems pretty dangerous! Oh well.
	int ret = std::system(std::format("{} -b -e {}", OPTIONS.mDolphinPath, (std::filesystem::path(OPTIONS.mRootPath) / "sys" / "main.dol").string()).c_str());
}

void LBooldozerEditor::SaveMapToFiles(std::string folder_path)
{
	if (mLoadedMap != nullptr)
		mLoadedMap->SaveMapToFiles(folder_path);
}

void LBooldozerEditor::SetGizmo(ImGuizmo::OPERATION mode)
{
	mCurrentMode->mGizmoMode = mode;
}

void LBooldozerEditor::ChangeMode()
{
	GetSelectionManager()->ClearSelection();
	if (mCurrentMode != nullptr)
		mCurrentMode->OnBecomeInactive();

	switch(CurrentMode)
	{
		case EEditorMode::Actor_Mode:
			mCurrentMode = &mActorMode;
			break;
		case EEditorMode::Door_Mode:
			mCurrentMode = &mDoorMode;
			break;
		case EEditorMode::Enemy_Mode:
			mCurrentMode = &mEnemyMode;
			break;
		case EEditorMode::Event_Mode:
			mCurrentMode = &mEventMode;
			break;
		case EEditorMode::Item_Mode:
			mCurrentMode = &mItemMode;
			break;
		case EEditorMode::Path_Mode:
			mCurrentMode = &mPathMode;
			break;
		case EEditorMode::Boo_Mode:
			mCurrentMode = &mBooMode;
			break;
		default:
			mCurrentMode = nullptr;
			break;
	}

	if (mCurrentMode != nullptr)
		mCurrentMode->OnBecomeActive();
}

void LBooldozerEditor::RenderNoRootPopup()
{
	if (ImGui::BeginPopupModal("Root Not Set", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("You currently do not have a valid Game Root set.\n");

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
