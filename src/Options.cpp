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

}

void LOptionsMenu::OpenMenu()
{
	mTempOptions = OPTIONS;
	ImGui::OpenPopup("Options");
}

void LOptionsMenu::RenderOptionsPopup(LEditorScene* scene)
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize({0.0f, 0.0f}, ImGuiCond_Always);

	if (ImGui::BeginPopupModal("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string path = "";

/*=== Dolphin Path ===*/
		// Text input box
		#ifdef _WIN32
		LUIUtility::RenderTextInput("Dolphin Path", &mTempOptions.mDolphinPath, 500);
		ImGui::SameLine(605);

		// Button for file dialog
		if (ImGui::Button("...##Dolphin")){
			ImGuiFileDialog::Instance()->OpenModal("SetDolphinPath", "Choose Dolphin Installation", "Executables (*.exe){.exe}", mTempOptions.mDolphinPath);
		}
		#else
		LUIUtility::RenderTextInput("Dolphin Command (ex, dolphin-emu)", &mTempOptions.mDolphinPath, 500);
		ImGui::SameLine(605);
		#endif
		
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
