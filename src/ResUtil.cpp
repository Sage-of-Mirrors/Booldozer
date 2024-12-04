#include "ResUtil.hpp"
#include "Options.hpp"
#include "imgui.h"
#include <format>

#include <glad/glad.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#include <stb_image.h>

LResUtility::LGCResourceManager GCResourceManager;

std::map<std::string, nlohmann::ordered_json> LResUtility::NameMaps = {};
std::array<uint32_t, 15> LResUtility::MapThumbnails = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

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
			std::cout << "[ResUtil] Loaded Game Archive" << std::endl;
			mLoadedGameArchive = true;
		}
	} else {
		std::cout << "[ResUtil]: Couldn't find game archive" << std::endl;
	}

	if(std::filesystem::exists(std::filesystem::current_path() / NAMES_BASE_PATH / "MapNames.json")){
		GetNameMap("MapNames");
	} else {
		NameMaps["MapNames"] = nlohmann::json();
		NameMaps["MapNames"]["names"] = nlohmann::json::array();
		for(int id = 0; id <= 14; id++){
			NameMaps["MapNames"]["names"][id] = std::format("Map {}", id);
		}
	}
	for(int id = 0; id <= 14; id++){
		MapThumbnails[id] = 0xFFFFFFFF;
	}

	std::filesystem::path bannerPath(std::filesystem::path(OPTIONS.mRootPath) / "files" / "opening.bnr");
	if(std::filesystem::exists(bannerPath)){
		bStream::CFileStream bnr(bannerPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
		bnr.readBytesTo((uint8_t*)&mBanner, sizeof(mBanner));
		bnr.seek(0x20);
		ImageFormat::Decode::RGB5A3(&bnr, 96, 32, mBannerImage);
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

void LResUtility::SetNameMap(std::string name, nlohmann::ordered_json json){
	NameMaps[name] = json;
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


std::tuple<std::string, std::string, bool> LResUtility::GetActorModelFromName(std::string name, bool log){
	auto names = GetNameMap("ActorNames");

	if (names.find(name) == names.end())
	{
		if(log) std::cout << std::format("[ResUtil]: Actor {0} not found in actor data\n", name);
		return {name, "", false};
	}

	bool fromGameArchive = names.at(name).size() == 3;

	return { names.at(name)[0], names.at(name)[1],  fromGameArchive };
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

// Thumbnails
void LResUtility::LoadMapThumbnails(){
    for(int id = 0; id <= 14; id++){
		int w, h, c;
		unsigned char* defaultProjImg = nullptr;
		
		if(id != 14){
			if(!std::filesystem::exists(std::filesystem::current_path() / RES_BASE_PATH / "thumb" / std::format("map{}.png", id))){
				continue;
			}
			defaultProjImg = stbi_load((std::filesystem::current_path() / RES_BASE_PATH / "thumb" / std::format("map{}.png", id)).string().c_str(), &w, &h, &c, 4);
		} else {
			defaultProjImg = stbi_load((std::filesystem::current_path() / RES_BASE_PATH / "thumb" / "default_thumb.png").string().c_str(), &w, &h, &c, 4);
		}

		uint32_t thumbId;
		glGenTextures(1, &thumbId);
		glBindTexture(GL_TEXTURE_2D, thumbId);
				
		MapThumbnails[id] = (thumbId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultProjImg);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(defaultProjImg);
	}
}

uint32_t LResUtility::GetMapThumbnail(uint32_t map) {
	return (MapThumbnails[map] == 0xFFFFFFFF ? MapThumbnails[14]: MapThumbnails[map]);
}

void LResUtility::SaveMapThumbnail(uint32_t w, uint32_t h, uint32_t map){
	if(!std::filesystem::exists(std::filesystem::current_path() / RES_BASE_PATH / "thumb")){
		std::filesystem::create_directory(std::filesystem::current_path() / RES_BASE_PATH / "thumb");
	}

	unsigned char* imgData = new unsigned char[w * h * 4]{0};
	unsigned char* imgDataScaled = new unsigned char[84 * 64 *4] {0};

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, imgData);

	stbir_resize_uint8_linear(imgData, w, h, 0, imgDataScaled, 84, 64, 0, STBIR_RGBA_NO_AW);

	stbi_write_png((std::filesystem::current_path() / RES_BASE_PATH / "thumb" / std::format("map{}.png", map)).string().c_str(), 84, 64, 4,  imgDataScaled, 84 * 4);
	delete imgData;
	delete imgDataScaled;
}

void LResUtility::CleanupThumbnails(){
	for(int id = 0; id <= 14; id++){
		glDeleteTextures(1, &MapThumbnails[id]);
	}
}