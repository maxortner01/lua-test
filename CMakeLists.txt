cmake_minimum_required(VERSION 3.8)

project(main)

set(CMAKE_CXX_STANDARD 17)

find_package(Lua REQUIRED)

# Packages for collision detection
find_package(Eigen3 REQUIRED)
find_package(ccd REQUIRED)

# Crap for fcl dependencies
find_path(CCD_INCLUDE_DIR ccd/ccd.h
    HINTS "${PC_CCD_INCLUDE_DIRS}"
          "${PC_CCD_INCLUDE_DIRS}/.."
          ENV PKG_CONFIG_SYSTEM_INCLUDE_PATH
          ENV C_INCLUDE_PATH
          ENV CPLUS_INCLUDE_PATH)

# Using find_library() even if pkg-config is available ensures that the full
# path to the ccd library is available in CCD_LIBRARIES
find_library(CCD_LIBRARY ccd
HINTS "${PC_CCD_LIBRARY_DIRS}")

# libccd links to LibM on UNIX.
if(CYGWIN OR NOT WIN32)
find_library(M_LIBRARY m)
endif()

if(CCD_INCLUDE_DIR AND CCD_LIBRARY)
    set(CCD_INCLUDE_DIRS "${CCD_INCLUDE_DIR}")
    set(CCD_LIBRARIES "${CCD_LIBRARY}" "${M_LIBRARY}")
    set(ccd_FOUND ON)

    mark_as_advanced(CCD_INCLUDE_DIR CCD_LIBRARY)
endif()

include_directories(
    ${LUA_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${CCD_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/extern/spdlog/include
    ${CMAKE_SOURCE_DIR}/extern/flecs/include
    ${CMAKE_SOURCE_DIR}/extern/sfml/include
    ${CMAKE_SOURCE_DIR}/extern/fcl/include
    ${CMAKE_CURRENT_BINARY_DIR}/extern/fcl/include)

link_libraries(${LUA_LIBRARIES})

## LUA
set(LUA_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Lua/Lib.cpp
    ${CMAKE_SOURCE_DIR}/src/Lua/Runtime.cpp
    ${CMAKE_SOURCE_DIR}/src/Lua/Table.cpp)

add_library(simple2d-lua SHARED ${LUA_SOURCES})

# ENGINE
set(BUILD_TESTING OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/extern/spdlog)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/flecs)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/sfml)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/fcl)

set(ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaScene.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Resources.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Application.cpp

    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/Surface.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/Math.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/Input.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/Time.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/World.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/Entity.cpp

    ${CMAKE_SOURCE_DIR}/src/Engine/Core/Core.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Core/Collide.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Core/Render.cpp

    ${CMAKE_SOURCE_DIR}/src/Engine/Components.cpp
    ${CMAKE_SOURCE_DIR}/src/Log/Library.cpp
    ${CMAKE_SOURCE_DIR}/src/Log/Log.cpp)

add_library(simple2d-engine STATIC ${ENGINE_SOURCES})

target_link_libraries(simple2d-engine PRIVATE simple2d-lua spdlog::spdlog fcl flecs sfml-graphics sfml-system sfml-window)

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