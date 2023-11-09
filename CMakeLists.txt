cmake_minimum_required(VERSION 3.8)

project(main)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${CMAKE_SOURCE_DIR}/extern/spdlog)

find_package(Lua REQUIRED)

add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")
include_directories(
    ${LUA_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/extern/spdlog/include)

add_executable(main main.cpp)

target_link_libraries(main PUBLIC ${LUA_LIBRARIES} spdlog::spdlog)