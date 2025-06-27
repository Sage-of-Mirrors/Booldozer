#pragma once

#include <filesystem>

extern std::filesystem::path RES_BASE_PATH;
extern std::filesystem::path NAMES_BASE_PATH;
extern std::filesystem::path USER_DATA_PATH;

void InitResourcePaths();
