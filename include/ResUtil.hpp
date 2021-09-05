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

namespace LResUtility
{
	nlohmann::json DeserializeJSON(std::filesystem::path file_path);

	nlohmann::json GetMapRoomNames(std::string mapName);

	uint32_t GetStaticMapDataOffset(std::string mapName, std::string region);

	std::filesystem::path GetStaticMapDataPath(std::string mapName);

	class LGCResourceManager
	{
		bool mInitialized = false;
		GCcontext mResManagerContext;
		public:
			bool LoadArchive(const char* path, GCarchive* archive);
			void Init();
	};

	void LoadUserSettings();
	void SaveUserSettings();
}

extern LResUtility::LGCResourceManager GCResourceManager;