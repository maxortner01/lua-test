cmake_minimum_required(VERSION 3.8)

project(main)

set(CMAKE_CXX_STANDARD 17)

find_package(Lua REQUIRED)
include_directories(
    ${LUA_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/extern/spdlog/include
    ${CMAKE_SOURCE_DIR}/extern/flecs/include
    ${CMAKE_SOURCE_DIR}/extern/sfml/include)

link_libraries(${LUA_LIBRARIES})

## LUA
set(LUA_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Lua/Lib.cpp
    ${CMAKE_SOURCE_DIR}/src/Lua/Runtime.cpp
    ${CMAKE_SOURCE_DIR}/src/Lua/Table.cpp)

add_library(simple2d-lua SHARED ${LUA_SOURCES})

# ENGINE
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/spdlog)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/flecs)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/sfml)

set(ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Engine/Resources.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Time.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/World.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Core.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Components.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/Log/Library.cpp
    ${CMAKE_SOURCE_DIR}/src/Log/Log.cpp)

add_library(simple2d-engine SHARED ${ENGINE_SOURCES})

target_link_libraries(simple2d-engine PRIVATE simple2d-lua spdlog::spdlog flecs sfml-graphics sfml-system sfml-window)

## EXECUTABLE
add_executable(main entry.cpp)

target_compile_definitions(main PUBLIC -DSOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(main PUBLIC simple2d-lua simple2d-engine flecs)

#set_target_properties(simple2d-engine simple2d-lua main
#    PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
#    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
#    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
#)