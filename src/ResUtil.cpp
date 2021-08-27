#include "ResUtil.hpp"

nlohmann::json LResUtility::DeserializeJSON(std::filesystem::path file_path)
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

nlohmann::json LResUtility::GetMapRoomNames(std::string mapName)
{
	std::string fileName = LGenUtility::Format(mapName, "_rooms.json");
	std::filesystem::path fullPath = std::filesystem::current_path() / NAMES_BASE_PATH / fileName;

	return DeserializeJSON(fullPath);
}

uint32_t LResUtility::GetStaticMapDataOffset(std::string mapName, std::string region)
{
	std::filesystem::path fullPath = std::filesystem::current_path() / RES_BASE_PATH / "static_room_data.json";
	nlohmann::json deserializedJson = DeserializeJSON(fullPath);

	if (deserializedJson.find(mapName) == deserializedJson.end())
	{
		std::cout << LGenUtility::Format("Map ", mapName, " not found in static room data at ", fullPath);
		return 0;
	}

	if (deserializedJson[mapName].find(region) == deserializedJson[mapName].end())
	{
		std::cout << LGenUtility::Format("Map ", mapName, " does not have static room data for region ", region);
		return 0;
	}

	return deserializedJson[mapName][region];
}
