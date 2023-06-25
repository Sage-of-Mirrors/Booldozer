#include "ResUtil.hpp"
#include "Options.hpp"
#include "imgui.h"
#include "fmt/core.h"

LResUtility::LGCResourceManager GCResourceManager;
std::map<std::string, nlohmann::ordered_json> LResUtility::NameMaps = {};

void LResUtility::LGCResourceManager::Init()
{
	GCerror err;
	if ((err = gcInitContext(&mResManagerContext)) != GC_ERROR_SUCCESS)
	{
		printf("Error initing arc loader context: %s\n", gcGetErrorMessage(err));
	}

	mInitialized = true;

	std::filesystem::path gameArcPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "Game" / "game_usa.szp";
	if(!GCResourceManager.LoadArchive(gameArcPath.string().data(), &mGameArchive)){
		std::cout << "Unable to load game archive " << gameArcPath.string() << std::endl;
	}
}

bool LResUtility::LGCResourceManager::LoadArchive(const char* path, GCarchive* archive)
{
	if(!mInitialized) return false;
	
	GCerror err;

	FILE* f = fopen(path, "rb");
	if (f == nullptr)
	{
		printf("Error opening file \"%s\"\n", path);
		return false;
	}

	fseek(f, 0L, SEEK_END);
	GCsize size = (GCsize)ftell(f);
	rewind(f);

	void* file = malloc(size);
	if (file == nullptr)
	{
		printf("Error allocating buffer for file \"%s\"\n", path);
		return false;
	}

	fread(file, 1, size, f);
	fclose(f);

	// If the file starts with 'Yay0', it's Yay0 compressed.
	if (*((uint32_t*)file) == 0x30796159)
	{
		GCsize compressedSize = gcDecompressedSize(&mResManagerContext, (GCuint8*)file, 0);

		void* decompBuffer = malloc(compressedSize);
		gcYay0Decompress(&mResManagerContext, (GCuint8*)file, (GCuint8*)decompBuffer, compressedSize, 0);

		free(file);
		file = decompBuffer;
	}
	// Likewise, if the file starts with 'Yaz0' it's Yaz0 compressed.
	else if (*((uint32_t*)file) == 0x307A6159)
	{
		GCsize compressedSize = gcDecompressedSize(&mResManagerContext, (GCuint8*)file, 0);

		void* decompBuffer = malloc(compressedSize);
		gcYaz0Decompress(&mResManagerContext, (GCuint8*)file, (GCuint8*)decompBuffer, compressedSize, 0);

		free(file);
		file = decompBuffer;
	}

	gcInitArchive(archive, &mResManagerContext);
	if ((err = gcLoadArchive(archive, file, size)) != GC_ERROR_SUCCESS) {
		printf("Error Loading Archive: %s\n", gcGetErrorMessage(err));
		return false;
	}

	return true;
}

GCarcfile* LResUtility::LGCResourceManager::GetFile(GCarchive* archive, std::filesystem::path filepath){
	int dirID = 0;
	for(std::string component : filepath){
		for (GCarcfile* file = &archive->files[archive->dirs[dirID].fileoff]; file < &archive->files[archive->dirs[dirID].fileoff] + archive->dirs[dirID].filenum; file++){
			if(strcmp(file->name, component.c_str()) == 0 && (file->attr & 0x02)){
				dirID = file->size;
				break;
			} else if(strcmp(file->name, component.c_str()) == 0 && !(file->attr & 0x02)) {
				return file;
			}
		}
	}
	return nullptr;
}

nlohmann::ordered_json LResUtility::DeserializeJSON(std::filesystem::path file_path)
{
	nlohmann::ordered_json j;

	if (file_path.empty() || !std::filesystem::exists(file_path))
	{
		std::cout << fmt::format("Unable to load JSON file from {0}", file_path.string()) << std::endl;
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

	std::filesystem::path fullPath = std::filesystem::current_path() / NAMES_BASE_PATH / fmt::format("{0}.json", name);

	auto json = DeserializeJSON(fullPath);
	if (!json.empty())
		NameMaps.emplace(name, json);

	return json;
}

nlohmann::ordered_json LResUtility::GetMirrorTemplate(std::string name)
{
	std::filesystem::path fullPath = std::filesystem::current_path() / RES_BASE_PATH / LGenUtility::Format(name, ".json");

	return DeserializeJSON(fullPath);
}

uint32_t LResUtility::GetStaticMapDataOffset(std::string mapName, std::string region)
{
	std::filesystem::path fullPath = std::filesystem::current_path() / RES_BASE_PATH / "static_room_data.json";
	nlohmann::json deserializedJson = DeserializeJSON(fullPath);

	if (deserializedJson.find(mapName) == deserializedJson.end())
	{
		//std::cout << LGenUtility::Format("Map ", mapName, " not found in static room data at ", fullPath);
		return 0;
	}

	if (deserializedJson[mapName].find(region) == deserializedJson[mapName].end())
	{
		//std::cout << LGenUtility::Format("Map ", mapName, " does not have static room data for region ", region);
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
		std::cout << fmt::format("Error saving user settings to {0}", fullPath.string());
}
