cmake_minimum_required(VERSION 3.8)
project(booldozer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib/bigg)
add_subdirectory(lib/libgctools)

add_executable(booldozer src/main.cpp)
target_include_directories(booldozer PUBLIC include)
target_link_libraries( booldozer PUBLIC bigg )