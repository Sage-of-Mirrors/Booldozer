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
	extern std::map<std::string, nlohmann::json> NameMaps;

	nlohmann::json DeserializeJSON(std::filesystem::path file_path);

	nlohmann::json GetNameMap(std::string name);

	uint32_t GetStaticMapDataOffset(std::string mapName, std::string region);

	std::filesystem::path GetStaticMapDataPath(std::string mapName);

	class LGCResourceManager
	{
		bool mInitialized = false;
		GCcontext mResManagerContext;
		public:
			GCarchive mGameArchive;
			bool LoadArchive(const char* path, GCarchive* archive);
			void Init();
	};

	void LoadUserSettings();
	void SaveUserSettings();
}

extern LResUtility::LGCResourceManager GCResourceManager;