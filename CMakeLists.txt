cmake_minimum_required(VERSION 3.8)
project(booldozer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

file(GLOB BOOLDOZER_SRC 
    "src/*.cpp"
    "include/*.hpp"
    "include/*.h"
    "src/ui/*.cpp"
    "include/ui/*.hpp"
    "src/DOM/*.cpp"
    "include/DOM/*.hpp"
    "src/modes/*.cpp"
    "include/modes/*.hpp"
    "src/history/*.cpp"
    "include/history/*.hpp"
    "src/scene/*.cpp"
    "include/scene/*.hpp"
    "src/io/*.cpp"
    "include/io/*.hpp"
    "include/nlohmann/*.hpp"
    "include/ImGuiFileDialog/*.h"
)

add_executable(booldozer ${BOOLDOZER_SRC})
find_package(ICU 61.0 COMPONENTS uc i18n REQUIRED)
target_include_directories(booldozer PUBLIC include lib/glfw/include)
target_link_libraries( booldozer PUBLIC imgui glfw gctools ICU::uc ICU::i18n)