#include "ui/BooldozerEditor.hpp"
#include "io/JmpIO.hpp"
#include "../lib/libgctools/include/compression.h"
#include "../lib/bStream/bstream.h"
#include "DOM/FurnitureDOMNode.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "imgui.h"
#include "ResUtil.hpp"
#include "UIUtil.hpp"
#include "DOL.hpp"
#include "io/StaticMapDataIO.hpp"
#include "Options.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"

LBooldozerEditor::LBooldozerEditor()
{
	CurrentMode = EEditorMode::Actor_Mode;
	mCurrentMode = &mActorMode;

	LResUtility::LoadUserSettings();
}

void LBooldozerEditor::Render(float dt, LEditorScene* renderer_scene)
{
	std::string path = "";

	if (LUIUtility::RenderFileDialog("OpenMapDlg", path))
	{
		OpenMap(path);

		OPTIONS.mLastOpenedMap = path;
		LResUtility::SaveUserSettings();
	}

	if (LUIUtility::RenderFileDialog("SaveMapFilesDlg", path))
	{
		SaveMapToFiles(path);

		OPTIONS.mLastSavedDirectory = path;
		LResUtility::SaveUserSettings();
	}

	mGhostConfigs.RenderUI();

	if (mLoadedMap != nullptr && !mLoadedMap->Children.empty() && mCurrentMode != nullptr)
		mCurrentMode->Render(mLoadedMap, renderer_scene);

	// Popups
	RenderNoRootPopup();
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
	//mGhostConfigs.LoadConfigs(mLoadedMap);
	//mLoadedMap->LoadMap(std::filesystem::path("/home/spacey/Projects/LuigisMansion/Mods/LMArcade/files/Map/map2.szp")); /* Space */
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

void LBooldozerEditor::onPlaytestCB()
{
	std::string args = LGenUtility::Format('\"', '\"', OPTIONS.mDolphinPath, "\" -b -e ", '\"', OPTIONS.mRootPath, "\\sys", "\\main.dol\"", '\"');
	int ret = system(args.c_str());
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
			break;
		case EEditorMode::Enemy_Mode:
			mCurrentMode = &mEnemyMode;
			break;
		case EEditorMode::Event_Mode:
			break;
		case EEditorMode::Item_Mode:
			mCurrentMode = &mItemMode;
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
