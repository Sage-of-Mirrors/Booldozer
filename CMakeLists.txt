cmake_minimum_required(VERSION 3.8)
project(booldozer)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(-DGLM_ENABLE_EXPERIMENTAL -DIMGUI_DEFINE_MATH_OPERATORS)
add_subdirectory(lib)

file(GLOB BOOLDOZER_SRC
    "src/*.cpp"
    "src/*.c"
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

    "lib/bsdifflib/bsdifflib.c"
    "lib/bsdifflib/bspatchlib.c"

    # glad
    "lib/glad/src/*.c"
)


add_executable(booldozer ${BOOLDOZER_SRC})
target_include_directories(booldozer PUBLIC include lib/libflipper/include lib/libflipper/include/geometry lib/imgui lib/imgui/backends lib/glfw/include lib/glad/include lib/glm lib/J3DUltra/include lib/gctoolsplusplus/include lib/TriStripper/include)

find_package(ICU 61.0 COMPONENTS uc i18n REQUIRED)
find_package(BZip2 REQUIRED)
target_link_libraries(booldozer PUBLIC imgui glfw gctools++ ICU::uc ICU::i18n BZip2::BZip2 j3dultra TriStripper)
