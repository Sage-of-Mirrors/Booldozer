#include "ui/BooldozerEditor.hpp"
#include <io/JmpIO.hpp>
#include <../lib/libgctools/include/compression.h>
#include <../lib/bStream/bstream.h>
#include <iostream>
#include <vector>
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
}

void LBooldozerEditor::Render(float dt, LEditorScene* renderer_scene)
{
	std::string path = "";

	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_AutoHideTabBar;
	mMainDockSpaceID = ImGui::DockSpaceOverViewport(mainViewport, dockFlags);
	
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
	}

	if (LUIUtility::RenderFileDialog("OpenMapDlg", path))
	{
		GetSelectionManager()->ClearSelection();
		OpenMap(path);

		OPTIONS.mLastOpenedMap = path;
		LResUtility::SaveUserSettings();
	}

	if (LUIUtility::RenderFileDialog("SaveMapArchiveDlg", path))
	{
		SaveMapToArchive(path);

		OPTIONS.mLastOpenedMap = path;
		LResUtility::SaveUserSettings();
	}

	if (LUIUtility::RenderFileDialog("SaveMapFilesDlg", path))
	{
		SaveMapToFiles(path);

		OPTIONS.mLastSavedDirectory = path;
		LResUtility::SaveUserSettings();
	}


	if (mLoadedMap != nullptr && !mLoadedMap->Children.empty() && mCurrentMode != nullptr)
		mCurrentMode->Render(mLoadedMap, renderer_scene);

	mGhostConfigs.RenderUI();
	
	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::Begin("viewportWindow");

	//TODO: check if window size changes so texture can be resized
	ImVec2 winSize = ImGui::GetContentRegionAvail();
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();

	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

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

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glViewport(0, 0, (uint32_t)winSize.x, (uint32_t)winSize.y);
	}

	glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int32_t unused = -1;
	glClearTexImage(mPickTex, 0, GL_RED_INTEGER, GL_INT, &unused);

	renderer_scene->RenderSubmit((uint32_t)winSize.x,  (uint32_t)winSize.y);


	ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(mViewTex)), winSize, {0.0f, 1.0f}, {1.0f, 0.0f});

	if(ImGui::IsItemClicked() && mLoadedMap != nullptr && !ImGuizmo::IsOver()){
		int32_t id;
		ImVec2 mousePos = ImGui::GetMousePos();

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glReadPixels(mousePos.x - cursorPos.x, ((uint32_t)winSize.y) - (mousePos.y - cursorPos.y), 1, 1, GL_RED_INTEGER, GL_INT, (void*)&id);

		std::cout << "Read clicked id from " << mousePos.x - cursorPos.x << " " << mousePos.y - cursorPos.y << " as " << id << std::endl;

		if(ImGui::IsKeyDown(ImGuiKey_LeftShift)) GetSelectionManager()->ClearSelection();

		for(auto node : mLoadedMap->GetChildrenOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender)){
			if(node->GetID() == id){
				GetSelectionManager()->AddToSelection(node);
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
	if (OPTIONS.mRootPath == "")
	{
		ImGui::OpenPopup("Root Not Set");
		return;
	}

	mLoadedMap = std::make_shared<LMapDOMNode>();
	mLoadedMap->LoadMap(std::filesystem::path(file_path));
	if(mLoadedMap == nullptr || mLoadedMap->Children.empty()){
		if (ImGui::BeginPopupModal("Map failed to load", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("No idea why, sorry boss.");
			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}
	mGhostConfigs.LoadConfigs(mLoadedMap);
}

void LBooldozerEditor::SaveMapToArchive(std::string file_path){
	if (OPTIONS.mRootPath == "")
	{
		ImGui::OpenPopup("Root Not Set");
		return;
	}

	mLoadedMap->SaveMapToArchive(file_path);
}

void LBooldozerEditor::onOpenMapCB()
{
	ImGuiFileDialog::Instance()->OpenDialog("OpenMapDlg", "Open map archive", "Archives (*.arc *.szs *.szp){.arc,.szs,.szp}", OPTIONS.mLastOpenedMap);
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "User selected open room(s)!" << std::endl;
}

void LBooldozerEditor::onSaveMapCB()
{
	if (mLoadedMap != nullptr)
		ImGuiFileDialog::Instance()->OpenDialog("SaveMapFilesDlg", "Choose a Folder", nullptr, OPTIONS.mLastSavedDirectory);
}

void LBooldozerEditor::onSaveMapArchiveCB()
{
	if (mLoadedMap != nullptr)
		ImGuiFileDialog::Instance()->OpenDialog("SaveMapArchiveDlg", "Choose an Archive", "Archives (*.szp){.szp}", OPTIONS.mLastOpenedMap);
}


void LBooldozerEditor::onPlaytestCB()
{
	//std::string args = LGenUtility::Format('\"', '\"', OPTIONS.mDolphinPath, "\" -b -e ", '\"', OPTIONS.mRootPath, "\\sys", "\\main.dol\"", '\"');
	//int ret = system(args.c_str());
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
		ImGui::Text("You currently do not have a valid Game Root set.\nYou must specify a copy of the game to work on before you can open maps.\n\nPlease open the Options menu (Edit -> Options) and provide a valid Game Root path.");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}
