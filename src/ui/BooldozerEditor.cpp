#include "ui/BooldozerEditor.hpp"
#include "JmpIO.hpp"
#include "../lib/libgctools/include/compression.h"
#include "../lib/bStream/bstream.h"
#include "DOM/FurnitureDOMNode.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "imgui.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"

LBooldozerEditor::LBooldozerEditor()
{
	CurrentMode = EEditorMode::Actor_Mode;
	mCurrentMode = &mActorMode;
}

void LBooldozerEditor::Render(float dt, LEditorScene* renderer_scene)
{
	// Render file dialog for opening map
	if (ImGuiFileDialog::Instance()->Display("OpenMapDlg"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			OpenMap(filePathName);
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}

	// Render file dialog for saving map to files
	if (ImGuiFileDialog::Instance()->Display("SaveMapFilesDlg"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string folderPathName = ImGuiFileDialog::Instance()->GetFilePathName();
			SaveMapToFiles(folderPathName);
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}

	if (mLoadedMap == nullptr || mLoadedMap->Children.empty() || mCurrentMode == nullptr)
		return;

	mCurrentMode->Render(mLoadedMap, renderer_scene);
}

void LBooldozerEditor::OpenMap(std::string file_path)
{
	mLoadedMap = std::make_shared<LMapDOMNode>();
	mLoadedMap->LoadMap(std::filesystem::path(file_path));

	//mLoadedMap->LoadMap(std::filesystem::path("/home/spacey/Projects/LuigisMansion/Mods/LMArcade/files/Map/map2.szp")); /* Space */
}

void LBooldozerEditor::onOpenMapCB()
{
	ImGuiFileDialog::Instance()->OpenDialog("OpenMapDlg", "Open map archive", "Archives (*.arc *.szs *.szp){.arc,.szs,.szp}", ".");
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "User selected open room(s)!" << std::endl;
}

void LBooldozerEditor::onSaveMapCB()
{
	if (mLoadedMap != nullptr)
		ImGuiFileDialog::Instance()->OpenDialog("SaveMapFilesDlg", "Choose a Folder", nullptr, ".");
}

void LBooldozerEditor::SaveMapToFiles(std::string folder_path)
{
	if (mLoadedMap != nullptr)
		mLoadedMap->SaveMapToFiles(folder_path);
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
