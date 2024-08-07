#pragma once

#include "GenUtil.hpp"
#include "constants.hpp"

#include <json.hpp>
#include <Archive.hpp>

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
	std::tuple<std::string, std::string, bool> GetActorModelFromName(std::string name);

	class LGCResourceManager
	{
		bool mInitialized = false;
		public:
			bool mLoadedGameArchive = false;
			std::shared_ptr<Archive::Rarc> mGameArchive { nullptr };
			void Init();
			void Cleanup();
	};

	void LoadUserSettings();
	void SaveUserSettings();
}

extern LResUtility::LGCResourceManager GCResourceManager;