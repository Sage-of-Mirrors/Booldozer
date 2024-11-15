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
#include <glad/glad.h>

#include <bzlib.h>
#include "../lib/bsdifflib/bspatchlib.h"

#include <cstdlib>

#include "DOM/CameraAnimationDOMNode.hpp"

namespace {
	char* patchErrorMsg { nullptr };
	std::thread mapOperationThread {};
	std::mutex loadLock {};
	bool mapLoading { false };
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

		ImGui::DockBuilderRemoveNode(mMainDockSpaceID); // clear any previous layout
		ImGui::DockBuilderAddNode(mMainDockSpaceID, dockFlags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(mMainDockSpaceID, mainViewport->Size);


		mDockNodeLeftID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Left, 0.25f, nullptr, &mMainDockSpaceID);
		mDockNodeRightID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Right, 0.25f, nullptr, &mMainDockSpaceID);
		mDockNodeDownLeftID = ImGui::DockBuilderSplitNode(mDockNodeLeftID, ImGuiDir_Down, 0.5f, nullptr, &mDockNodeUpLeftID);

		ImGui::DockBuilderDockWindow("sceneHierarchy", mDockNodeUpLeftID);
		ImGui::DockBuilderDockWindow("detailWindow", mDockNodeDownLeftID);
		ImGui::DockBuilderDockWindow("toolWindow", mDockNodeRightID);
		ImGui::DockBuilderDockWindow("viewportWindow", mMainDockSpaceID);

		ImGui::DockBuilderFinish(mMainDockSpaceID);
		bInitialized = true;

		GCResourceManager.Init();
		renderer_scene->LoadResFromRoot();
		
		if (OPTIONS.mRootPath == "")
		{
			ImGui::OpenPopup("Root Not Set");
		} else if(OPTIONS.mRootPath != "" && !OPTIONS.mIsDOLPatched){
			ImGui::OpenPopup("Unpatched DOL");
		}
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

	if (ImGui::BeginPopupModal("Unpatched DOL", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("This root has a clean DOL. Certain edits will not be reflected in game!");
		ImGui::Text("Apply externalized map data patch?");
		ImGui::Separator();
		if (ImGui::Button("Yes")) {
			std::filesystem::path patchPath = std::filesystem::current_path() / RES_BASE_PATH / "externalizemaps.patch";
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
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Loading Map", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		ImGui::AlignTextToFramePadding();
		ImGui::Spinner("##loadmapSpinner", 5.0f, 2, col);
		ImGui::SameLine();
		ImGui::Text("Loading Map...");
		
		loadLock.lock();
		if(mapLoading == false){
			std::cout << "[BooldozerEditor]: Joining load/append thread" << std::endl;
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

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Saving Map", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		ImGui::AlignTextToFramePadding();
		ImGui::Spinner("##loadmapSpinner", 5.0f, 2, col);
		ImGui::SameLine();
		ImGui::Text("Saving Map...");

		loadLock.lock();
		if(mapLoading == false){
			std::cout << "[BooldozerEditor]: Joining save thread" << std::endl;
			mapOperationThread.join();
			ImGui::CloseCurrentPopup();
		}
		loadLock.unlock();

		ImGui::EndPopup();
	}
		
	if (LUIUtility::RenderFileDialog("OpenMapDlg", path))
	{
		GetSelectionManager()->ClearSelection();
		renderer_scene->Clear();
		
		loadLock.lock();
		mapLoading = true;
		loadLock.unlock();

		mapOperationThread = std::thread(&LBooldozerEditor::LoadMap, std::ref(*this), path, renderer_scene);
		ImGui::OpenPopup("Loading Map");
	}

	if (LUIUtility::RenderFileDialog("AppendMapDlg", path))
	{
		GetSelectionManager()->ClearSelection();
		
		loadLock.lock();
		mapLoading = true;
		loadLock.unlock();

		mapOperationThread = std::thread(&LBooldozerEditor::AppendMap, std::ref(*this), path);
		ImGui::OpenPopup("Loading Map");
	}

	if (LUIUtility::RenderFileDialog("SaveMapArchiveDlg", path))
	{
		loadLock.lock();
		mapLoading = true;
		loadLock.unlock();

		mapOperationThread = std::thread(&LBooldozerEditor::SaveMap, std::ref(*this), path);
		ImGui::OpenPopup("Saving Map");
	}


	if (mapLoading == false && mLoadedMap != nullptr && !mLoadedMap->Children.empty() && mCurrentMode != nullptr)
		mCurrentMode->Render(mLoadedMap, renderer_scene);

	mGhostConfigs.RenderUI();
	
	CameraAnimation::RenderPreview();

	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::Begin("viewportWindow");

	//TODO: check if window size changes so texture can be resized
	ImVec2 winSize = ImGui::GetContentRegionAvail();
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();

	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mRbo);

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

	renderer_scene->SetActive(ImGui::IsItemHovered());

	if(ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) ImGui::FocusItem();

	if(ImGui::IsItemClicked() && mLoadedMap != nullptr && !ImGuizmo::IsOver()){
		int32_t id;
		ImVec2 mousePos = ImGui::GetMousePos();

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glReadPixels(mousePos.x - cursorPos.x, ((uint32_t)winSize.y) - (mousePos.y - cursorPos.y), 1, 1, GL_RED_INTEGER, GL_INT, (void*)&id);

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
			} else {
				if(node->GetID() == id){
					GetSelectionManager()->AddToSelection(node);
				}
			}
		}
	}

	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, winSize.x, winSize.y);

	mCurrentMode->RenderGizmo(renderer_scene);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ImGui::End();


	// Popups
	RenderNoRootPopup();



	//fbo copy
}

void LBooldozerEditor::OpenMap(std::string file_path)
{

	mLoadedMap = std::make_shared<LMapDOMNode>();
	std::cout << "[MapDOMNode] Starting Load Map" << std::endl;
	mLoadedMap->LoadMap(std::filesystem::path(file_path));

	mGhostConfigs.LoadConfigs(mLoadedMap);
}

void LBooldozerEditor::AppendMap(std::string map_path){
	auto appendMap = std::make_shared<LMapDOMNode>();
	if(appendMap->LoadMap(std::filesystem::path(map_path))){
		mLoadedMap->AppendMap(appendMap);
	}
	loadLock.lock();
	mapLoading = false;
	loadLock.unlock();
}

void LBooldozerEditor::SaveMapToArchive(std::string file_path){
	mLoadedMap->SaveMapToArchive(file_path);
}

void LBooldozerEditor::onOpenMapCB()
{
	ImGuiFileDialog::Instance()->OpenDialog("OpenMapDlg", "Open map archive", "Archives (*.arc *.szs *.szp){.arc,.szs,.szp}", OPTIONS.mLastOpenedMap);
}

void LBooldozerEditor::onAppendMapCB()
{
	ImGuiFileDialog::Instance()->OpenDialog("AppendMapDlg", "Open map archive", "Archives (*.arc *.szs *.szp){.arc,.szs,.szp}", OPTIONS.mLastOpenedMap);
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "[Booldozer]: User selected open room(s)!" << std::endl;
}

void LBooldozerEditor::onSaveMapCB()
{
	if (mLoadedMap != nullptr)
		ImGuiFileDialog::Instance()->OpenDialog("SaveMapFilesDlg", "Choose a Folder", nullptr, OPTIONS.mLastSavedDirectory);
}

void LBooldozerEditor::onSaveMapArchiveCB()
{
	if (mLoadedMap != nullptr)
		ImGuiFileDialog::Instance()->OpenDialog("SaveMapArchiveDlg", "Choose an Archive", "Archives (*.szp){.szp}", OPTIONS.mLastSavedDirectory);
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
		case EEditorMode::Collision_Mode:
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
			mOpenRootFlag = true;
		}
		ImGui::EndPopup();
	}
}
