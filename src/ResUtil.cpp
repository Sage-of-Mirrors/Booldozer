#include "ResUtil.hpp"
#include "Options.hpp"
#include "imgui.h"

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

bool LResUtility::LGCResourceManager::ReplaceArchiveFileData(GCarcfile* file, uint8_t* new_data, size_t new_data_size){
	if(!mInitialized) return false;
	
	// free existing file
	gcFreeMem(&mResManagerContext, file->data);

	//allocate size of new file
	file->data = gcAllocMem(&mResManagerContext, new_data_size);
		
	//copy new jmp to file buffer for arc
	memcpy(file->data, new_data, new_data_size);

	//set size properly
	file->size = new_data_size;

	return true;
}

bool LResUtility::LGCResourceManager::SaveArchiveCompressed(const char* path, GCarchive* archive)
{
	if(!mInitialized) return false;

	GCsize outSize = gcSaveArchive(archive, NULL);
	GCuint8* archiveOut = new GCuint8[outSize];
	GCuint8* archiveCmp = new GCuint8[outSize];

	gcSaveArchive(archive, archiveOut);
	GCsize cmpSize = gcYay0Compress(&mResManagerContext, archiveOut, archiveCmp, outSize);
	
	std::ofstream fileStream;
	fileStream.open(path, std::ios::binary | std::ios::out);
	fileStream.write((const char*)archiveCmp, cmpSize);
	fileStream.close();

	delete archiveOut;
	delete archiveCmp;

	return true;
}

nlohmann::ordered_json LResUtility::DeserializeJSON(std::filesystem::path file_path)
{
	nlohmann::ordered_json j;

	if (file_path.empty() || !std::filesystem::exists(file_path))
	{
		std::cout << LGenUtility::Format("Unable to load JSON file from ", file_path) << std::endl;
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

	std::filesystem::path fullPath = std::filesystem::current_path() / NAMES_BASE_PATH / LGenUtility::Format(name, ".json");

	auto json = DeserializeJSON(fullPath);
	if (!json.empty())
		NameMaps.emplace(name, json);

	return json;
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

std::filesystem::path LResUtility::GetStaticMapDataPath(std::string mapName)
{
	if (OPTIONS.mRootPath == "")
		return "";

	return std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / mapName / "rooms.map";
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
		std::cout << LGenUtility::Format("Error saving user settings to ", fullPath);
}
