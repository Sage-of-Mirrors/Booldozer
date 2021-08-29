#include "Options.hpp"
#include "UIUtil.hpp";
#include "ResUtil.hpp"

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

void LOptionsMenu::RenderOptionsPopup()
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		LUIUtility::RenderTextInput("Game Root", &mTempOptions.mRootPath);
		LUIUtility::RenderTooltip("This is the copy of the game that you are currently editing. All models, events, etc. will be loaded from here.");

		LUIUtility::RenderTextInput("Dolphin Path", &mTempOptions.mDolphinPath);
		LUIUtility::RenderTooltip("This is the installation of Dolphin that will be used to run playtests.");

		if (ImGui::Button("Save", ImVec2(120, 0)))
		{
			OPTIONS = mTempOptions;
			LResUtility::SaveUserSettings();

			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}
