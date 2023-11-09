cmake_minimum_required(VERSION 3.8)

project(main)

set(CMAKE_CXX_STANDARD 17)

find_package(Lua REQUIRED)

add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")
include_directories(
    ${LUA_INCLUDE_DIR})

add_executable(main main.cpp)

target_link_libraries(main PUBLIC ${LUA_LIBRARIES})