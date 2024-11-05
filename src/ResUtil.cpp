#include "ResUtil.hpp"
#include "Options.hpp"
#include "imgui.h"
#include <format>

LResUtility::LGCResourceManager GCResourceManager;
std::map<std::string, nlohmann::ordered_json> LResUtility::NameMaps = {};

void LResUtility::LGCResourceManager::Init()
{
	mInitialized = true;
	mGameArchive = Archive::Rarc::Create();

	std::filesystem::path gameArcPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "Game" / "game_usa.szp";
	std::cout << "[ResUtil] Loading Game Archive " << gameArcPath.string() << std::endl;
	if(std::filesystem::exists(gameArcPath)){
		bStream::CFileStream gameArchiveFileStream(gameArcPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
		if(!mGameArchive->Load(&gameArchiveFileStream)){
			std::cout << "[ResUtil]: Unable to load Game Archive " << gameArcPath.string() << std::endl;
			mLoadedGameArchive = false;
		} else {
			mLoadedGameArchive = true;
		}
	} else {
		std::cout << "[ResUtil]: Couldn't find game archive" << std::endl;
	}
}

void LResUtility::LGCResourceManager::Cleanup(){}

nlohmann::ordered_json LResUtility::DeserializeJSON(std::filesystem::path file_path)
{
	nlohmann::ordered_json j;

	if (file_path.empty() || !std::filesystem::exists(file_path))
	{
		std::cout << std::format("[ResUtil]: Unable to load JSON file from {0}", file_path.string()) << std::endl;
		return j;
	}

	std::ifstream srcFile(file_path);
	if (srcFile.is_open())
	{
		srcFile >> j;
	}

	return j;
}

nlohmann::ordered_json LResUtility::GetNameMap(std::string name)
{
	if (NameMaps.count(name) != 0)
		return NameMaps[name];

	std::filesystem::path fullPath = std::filesystem::current_path() / NAMES_BASE_PATH / std::format("{0}.json", name);

	auto json = DeserializeJSON(fullPath);
	if (!json.empty())
		NameMaps.emplace(name, json);

	return json;
}

nlohmann::ordered_json LResUtility::GetMirrorTemplate(std::string name)
{
	std::filesystem::path fullPath = std::filesystem::current_path() / RES_BASE_PATH / std::format("{0}.json", name);

	return DeserializeJSON(fullPath);
}

uint32_t LResUtility::GetStaticMapDataOffset(std::string mapName, std::string region)
{
	std::filesystem::path fullPath = std::filesystem::current_path() / RES_BASE_PATH / "static_room_data.json";
	nlohmann::json deserializedJson = DeserializeJSON(fullPath);

	if (deserializedJson.find(mapName) == deserializedJson.end())
	{
		std::cout << std::format("[ResUtil]: Map {0} not found in static room data at {1}\n", mapName, fullPath.string());
		return 0;
	}

	if (deserializedJson[mapName].find(region) == deserializedJson[mapName].end())
	{
		std::cout << std::format("[ResUtil]: Map {0} does not have static room data for region {1}\n", mapName, region);
		return 0;
	}

	return deserializedJson[mapName][region];
}

std::filesystem::path LResUtility::GetStaticMapDataPath(std::string mapName)
{
	if (OPTIONS.mRootPath == "")
		return "";

	return std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / mapName / "rooms.map";
}

std::filesystem::path LResUtility::GetMirrorDataPath(std::string mapName)
{
	if (OPTIONS.mRootPath == "")
		return "";

	return std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / mapName / "mirrors.bin";
}


std::tuple<std::string, std::string, bool> LResUtility::GetActorModelFromName(std::string name){
	std::filesystem::path fullPath = std::filesystem::current_path() / RES_BASE_PATH / "names" / "ActorNames.json";
	nlohmann::json deserializedJson = DeserializeJSON(fullPath);

	if (deserializedJson.find(name) == deserializedJson.end())
	{
		std::cout << std::format("[ResUtil]: Actor {0} not found in actor data\n", name);
		return {name, "", false};
	}

	bool fromGameArchive = deserializedJson.at(name).size() == 3;

	return { deserializedJson.at(name)[0], deserializedJson.at(name)[1],  fromGameArchive };
}

void LResUtility::LoadUserSettings()
{
	std::filesystem::path fullPath = std::filesystem::current_path() / "user_settings.json";

	// If the settings file doesn't exist, cause it to be created with the default settings.
	if (!std::filesystem::exists(fullPath))
	{
		SaveUserSettings();
		return;
	}

	nlohmann::json deserialized = DeserializeJSON(fullPath);
	LUserOptions::FromJson(deserialized, OPTIONS);
}

void LResUtility::SaveUserSettings()
{
	std::filesystem::path fullPath = std::filesystem::current_path() / "user_settings.json";
	nlohmann::json serialize;
	LUserOptions::ToJson(serialize, OPTIONS);

	std::ofstream destFile(fullPath);
	if (destFile.is_open())
		destFile << serialize;
	else
		std::cout << std::format("[ResUtil]: Error saving user settings to {0}", fullPath.string());
}
