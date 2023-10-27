cmake_minimum_required(VERSION 3.8)
project(booldozer)
set(CMAKE_CXX_STANDARD 20)
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
    
    # glad
    "lib/glad/src/*.c"
)


add_executable(booldozer ${BOOLDOZER_SRC})
target_include_directories(booldozer PUBLIC include lib/libflipper/include lib/libflipper/include/geometry lib/imgui lib/imgui/backends lib/fmt/include lib/glfw/include lib/glad/include lib/glm lib/J3DUltra/include)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)

find_package(ICU 61.0 COMPONENTS uc i18n REQUIRED)
target_link_libraries(booldozer PUBLIC imgui glfw gctools ICU::uc ICU::i18n fmt j3dultra)