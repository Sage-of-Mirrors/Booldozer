#include "Options.hpp"
#include "UIUtil.hpp"
#include "ResUtil.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"

#include <imgui.h>

LUserOptions OPTIONS;

LUserOptions::LUserOptions() : mRootPath(""), mDolphinPath(""), mIsDOLPatched(false), mLastOpenedMap("."), mLastSavedDirectory(".")
{

}

void LUserOptions::ToJson(nlohmann::json& j, LUserOptions options)
{
	j = nlohmann::json{
		{ "root_path", options.mRootPath },
		{ "dolphin_path", options.mDolphinPath },
		{ "is_dol_patched", options.mIsDOLPatched },
		{ "last_opened_map", options.mLastOpenedMap },
		{ "last_saved_directory", options.mLastSavedDirectory }
	};
}

void LUserOptions::FromJson(const nlohmann::json& j, LUserOptions& options)
{
	j.at("root_path").get_to(options.mRootPath);
	j.at("dolphin_path").get_to(options.mDolphinPath);
	j.at("is_dol_patched").get_to(options.mIsDOLPatched);
	j.at("last_opened_map").get_to(options.mLastOpenedMap);
	j.at("last_saved_directory").get_to(options.mLastSavedDirectory);

	DOL dol;
	dol.LoadDOLFile(std::filesystem::path(options.mRootPath) / "sys" / "main.dol");

}

void LOptionsMenu::OpenMenu()
{
	mTempOptions = OPTIONS;
	ImGui::OpenPopup("Options");
}

void LOptionsMenu::RenderOptionsPopup()
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string path = "";

/*=== Game Root ===*/
		// Text input box
		LUIUtility::RenderTextInput("Game Root", &mTempOptions.mRootPath, 500);
		ImGui::SameLine(605);
		
		// Button for folder dialog
		if (ImGui::Button("...##Root"))
			ImGuiFileDialog::Instance()->OpenModal("SetGameRoot", "Choose Game Root", nullptr, mTempOptions.mRootPath);

		// Render folder dialog if open
		if (LUIUtility::RenderFileDialog("SetGameRoot", path)){
			mTempOptions.mRootPath = path;
			GCResourceManager.Init(); // reinit resource manager to reload game archive
		}

		// Tooltip
		LUIUtility::RenderTooltip("This is the copy of the game that you are currently editing. All models, events, etc. will be loaded from here.");

/*=== Dolphin Path ===*/
		// Text input box
		LUIUtility::RenderTextInput("Dolphin Path", &mTempOptions.mDolphinPath, 500);
		ImGui::SameLine(605);

		// Button for file dialog
		if (ImGui::Button("...##Dolphin"))
			ImGuiFileDialog::Instance()->OpenModal("SetDolphinPath", "Choose Dolphin Installation", "Executables (*.exe){.exe}", mTempOptions.mDolphinPath);

		// Render file dialog if open
		if (LUIUtility::RenderFileDialog("SetDolphinPath", path))
			mTempOptions.mDolphinPath = path;

		// Tooltip
		LUIUtility::RenderTooltip("This is the installation of Dolphin that will be used to run playtests.");

/*=== Saving/Canceling ===*/
		ImGui::Separator();

		// Save button
		if (ImGui::Button("Save", ImVec2(120, 0)))
		{
			if(OPTIONS.mRootPath != mTempOptions.mRootPath){
				DOL dol;
				dol.LoadDOLFile(std::filesystem::path(mTempOptions.mRootPath) / "sys" / "main.dol");
				if(!OPTIONS.mIsDOLPatched){
					ImGui::OpenPopup("Unpatched DOL");
				}
			}
			OPTIONS = mTempOptions;
			LResUtility::SaveUserSettings();

			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		// Cancel button
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}
