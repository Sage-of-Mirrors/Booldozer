#pragma once

#include <json.hpp>

#include <string>

struct LUserOptions
{
	LUserOptions();

	// Visible to the user
	std::string mRootPath;
	std::string mDolphinPath;

	// Not visible to the user
	std::string mLastOpenedMap;
	std::string mLastSavedDirectory;
	bool mIsDOLPatched;

	static void ToJson(nlohmann::json& j, LUserOptions options);
	static void FromJson(const nlohmann::json& j, LUserOptions& options);
};

extern LUserOptions OPTIONS;

struct LOptionsMenu
{
	LUserOptions mTempOptions;

	void OpenMenu();
	void RenderOptionsPopup();
};
