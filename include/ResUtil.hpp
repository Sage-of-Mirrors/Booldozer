#pragma once

#include "GenUtil.hpp"
#include "constants.hpp"

#include <nlohmann/json.hpp>
#include "../lib/libgctools/include/archive.h"
#include "../lib/libgctools/include/compression.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>

namespace LResUtility
{
	extern std::map<std::string, nlohmann::ordered_json> NameMaps;

	nlohmann::ordered_json DeserializeJSON(std::filesystem::path file_path);

	nlohmann::ordered_json GetNameMap(std::string name);
	nlohmann::ordered_json GetMirrorTemplate(std::string name);

	uint32_t GetStaticMapDataOffset(std::string mapName, std::string region);

	std::filesystem::path GetStaticMapDataPath(std::string mapName);
	std::filesystem::path GetMirrorDataPath(std::string mapName);

	class LGCResourceManager
	{
		bool mInitialized = false;
		bool mLoadedGameArchive = false;
		GCcontext mResManagerContext;
		public:
			GCarchive mGameArchive;
			bool LoadArchive(const char* path, GCarchive* archive);
			bool SaveArchiveCompressed(const char* path, GCarchive* archive);
			bool ReplaceArchiveFileData(GCarcfile* file, uint8_t* new_data, size_t new_data_size);
			GCarcfile* GetFile(GCarchive* archive, std::filesystem::path filepath);
			void Init();
			void Cleanup();
	};

	void LoadUserSettings();
	void SaveUserSettings();
}

extern LResUtility::LGCResourceManager GCResourceManager;