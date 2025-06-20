#pragma once

#include <Bti.hpp>
#include "GenUtil.hpp"
#include "constants.hpp"

#include <json.hpp>
#include <Archive.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>

struct GCBanner {
	uint32_t mMagic;
	uint8_t mPadding[28];
	uint8_t mImage[0x1800];
	char mGameTitle[0x20];
	char mDeveloper[0x20];
	char mGameTitleFull[0x40];
	char mDeveloperFull[0x40];
	char mGameDescription[0x80];
};

namespace LResUtility
{
	extern std::map<std::string, nlohmann::ordered_json> NameMaps;

	extern std::array<uint32_t, 15> MapThumbnails;

	uint32_t GetMapThumbnail(uint32_t map);
	void SaveMapThumbnail(uint32_t w, uint32_t h, uint32_t map);
	void LoadMapThumbnails(std::string dir="");
	void CleanupThumbnails();

	nlohmann::ordered_json DeserializeJSON(std::filesystem::path file_path);

	nlohmann::ordered_json GetNameMap(std::string name);
	void SetNameMap(std::string name, nlohmann::ordered_json json);
	nlohmann::ordered_json GetMirrorTemplate(std::string name);

	uint32_t GetStaticMapDataOffset(std::string mapName, std::string region);

	std::filesystem::path GetStaticMapDataPath(std::string mapName);
	std::filesystem::path GetMirrorDataPath(std::string mapName);
	std::tuple<std::string, std::string, bool> GetActorModelFromName(std::string name, bool log = true);

	class LGCResourceManager
	{
		bool mInitialized = false;
		public:
			bool mLoadedGameArchive = false;
			std::shared_ptr<Archive::Rarc> mGameArchive { nullptr };
			
			GCBanner mBanner;
			uint8_t mBannerImage[96*32*4] { 0 };
			
			void Init();
			void Cleanup();
	};

	void LoadUserSettings();
	void SaveUserSettings();
}

extern LResUtility::LGCResourceManager GCResourceManager;