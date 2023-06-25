cmake_minimum_required(VERSION 3.8)
project(booldozer)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

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

find_package(Iconv REQUIRED)

add_executable(booldozer ${BOOLDOZER_SRC})
target_include_directories(booldozer PUBLIC include lib/fmt/include lib/glfw/include lib/glad/include lib/glm lib/libflipper/include lib/J3DUltra/include)
target_link_libraries(booldozer PUBLIC imgui glfw gctools libflipper ${ICONV_LIBRARIES} fmt j3dultra)