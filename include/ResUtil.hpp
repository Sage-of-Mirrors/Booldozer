#pragma once

#include "GenUtil.hpp"
#include "constants.hpp"

#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace LResUtility
{
	nlohmann::json DeserializeJSON(std::filesystem::path file_path);

	nlohmann::json GetMapRoomNames(std::string mapName);

	uint32_t GetStaticMapDataOffset(std::string mapName, std::string region);

	nlohmann::json GetUserSettings();
	void CreateUserSettings(nlohmann::json& settings);
	void SaveUserSettings(nlohmann::json settings);
}
