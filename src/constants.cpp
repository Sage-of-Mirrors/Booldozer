#include "constants.hpp"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#elif __linux__
#include <unistd.h>
#include <iostream>
#endif

std::filesystem::path RES_BASE_PATH = std::filesystem::current_path() / std::filesystem::path("res");
std::filesystem::path NAMES_BASE_PATH = std::filesystem::current_path() / RES_BASE_PATH / "names";
std::filesystem::path USER_DATA_PATH;

// Looks for system resource paths if they exist and the local resource path doesn't exist. Best for installations
void InitResourcePaths(){
#ifdef __linux__
    char *homeEnv = getenv("HOME");
    if (homeEnv == nullptr) {
        std::cerr << "Error getting home directory." << std::endl;
        return;
    }
    std::filesystem::path home(homeEnv);
    std::filesystem::create_directory(home / ".local/share/booldozer");
    USER_DATA_PATH = std::filesystem::path(home / ".local/share/booldozer");
    if(!std::filesystem::exists(RES_BASE_PATH)){
        if(!std::filesystem::exists(USER_DATA_PATH / "res")){
            std::filesystem::create_directory(USER_DATA_PATH / "res");
        }

        RES_BASE_PATH = std::filesystem::path(USER_DATA_PATH / "res");
        NAMES_BASE_PATH = RES_BASE_PATH / "names";
        if(!std::filesystem::exists(NAMES_BASE_PATH)){
            std::filesystem::create_directory(NAMES_BASE_PATH);
        }
    }
#elif _WIN32
    PWSTR pathTmp;

    auto folderPathRet = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathTmp);
    if (folderPathRet != S_OK) {
        CoTaskMemFree(pathTmp);
        return;
    }

    USER_DATA_PATH = pathTmp;
    USER_DATA_PATH = USER_DATA_PATH / "booldozer";
    if(!std::filesystem::exists(USER_DATA_PATH)){
        std::filesystem::create_directory(USER_DATA_PATH);
    }

    CoTaskMemFree(pathTmp);

    if(!std::filesystem::exists(RES_BASE_PATH)){
        RES_BASE_PATH = RES_BASE_PATH / "res";;
        if(!std::filesystem::exists(RES_BASE_PATH)){
            std::filesystem::create_directory(RES_BASE_PATH);
        }

        NAMES_BASE_PATH = RES_BASE_PATH / "names";
        if(!std::filesystem::exists(NAMES_BASE_PATH)){
            std::filesystem::create_directory(NAMES_BASE_PATH);
        }
    }
#endif
}
