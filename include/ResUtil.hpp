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
	nlohmann::json DeserializeJSON(std::filesystem::path file_path)
	{
		nlohmann::json j;

		if (file_path.empty() || !std::filesystem::exists(file_path))
		{
			std::cout << LGenUtility::Format("Unable to load JSON file from ", file_path);
			return j;
		}

		std::ifstream srcFile(file_path);
		if (srcFile.is_open())
		{
			srcFile >> j;
		}

		return j;
	}

	nlohmann::json GetMapRoomNames(std::string mapName)
	{
		std::string fileName = LGenUtility::Format(mapName, "_rooms.json");
		std::filesystem::path fullPath = std::filesystem::current_path() / NAMES_BASE_PATH / fileName;

		return DeserializeJSON(fullPath);
	}
}
