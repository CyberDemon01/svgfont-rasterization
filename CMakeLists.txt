cmake_minimum_required(VERSION 3.5)

project(raster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-Wall -Wextra -O3)

find_package(SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory("pugixml/")
include_directories("pugixml/src/")

add_executable(raster main.cpp)
target_link_libraries(raster SDL2::SDL2)
target_link_libraries(raster pugixml)
