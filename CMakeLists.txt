cmake_minimum_required(VERSION 3.8)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH OFF)

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
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/extern/spdlog/include
    ${CMAKE_SOURCE_DIR}/extern/flecs/include)

## LUA
set(LUA_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Lua/TypeMap.cpp
    ${CMAKE_SOURCE_DIR}/src/Lua/Lib.cpp
    ${CMAKE_SOURCE_DIR}/src/Lua/Runtime.cpp
    ${CMAKE_SOURCE_DIR}/src/Lua/Table.cpp)

add_library(simple2d-lua SHARED ${LUA_SOURCES})

target_link_libraries(simple2d-lua PRIVATE ${LUA_LIBRARIES})
target_include_directories(simple2d-lua PRIVATE ${LUA_INCLUDE_DIR})

set(BUILD_TESTING OFF)

## GRAPHICS
# Build SDL as a dll needed in the final product
set(SDL_TEST_LIBRARY OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/SDL)

# Build the static glew into graphics
set(glew-cmake_BUILD_SHARED OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glew-cmake)

# Build the static library into graphics
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/freetype)

set(GRAPHICS_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Log/Log.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Text.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Font.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Image.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Surface.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/DrawTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Texture.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/VertexArray.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Shader.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/DrawWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Keyboard.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Mouse.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics/Window.cpp)

#if USE_IMGUI
set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/extern/imgui/imconfig.h
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_internal.h
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui.h
    ${CMAKE_SOURCE_DIR}/extern/imgui/imstb_rectpack.h
    ${CMAKE_SOURCE_DIR}/extern/imgui/imstb_textedit.h
    ${CMAKE_SOURCE_DIR}/extern/imgui/imstb_truetype.h
    
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_opengl3.h
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_sdl3.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_sdl3.h)

set(GRAPHICS_INCLUDES ${CMAKE_SOURCE_DIR}/extern/freetype/include)

set(IMGUI_INCLUDES ${CMAKE_SOURCE_DIR}/extern/imgui)

add_library(simple2d-graphics SHARED ${GRAPHICS_SOURCES})

#target_compile_definitions(simple2d-graphics PRIVATE -DUSE_IMGUI)
target_include_directories(simple2d-graphics PRIVATE ${GRAPHICS_INCLUDES})
target_link_libraries(simple2d-graphics PRIVATE SDL3::SDL3 libglew_static freetype)

# ENGINE
# Engine needs lua for now
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/spdlog)

set(FLECS_STATIC OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/flecs)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/fcl)

set(ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaScene.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Mesh/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Resources.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/Renderer.cpp

    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/ImageLib.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/ResLib.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine/LuaLib/MeshLib.cpp
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

add_library(simple2d-engine SHARED ${ENGINE_SOURCES})

target_compile_definitions(simple2d-engine PRIVATE -DUSE_IMGUI)
target_link_libraries(simple2d-engine PRIVATE simple2d-lua simple2d-graphics spdlog::spdlog fcl flecs ${LUA_LIBRARIES})
target_include_directories(simple2d-engine PRIVATE 
    ${LUA_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${CCD_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/extern/fcl/include
    ${CMAKE_CURRENT_BINARY_DIR}/extern/fcl/include)

## APPLICATION
set(APPLICATION_SOURCES ${CMAKE_SOURCE_DIR}/src/Engine/Application.cpp)

add_library(simple2d-entry STATIC ${APPLICATION_SOURCES})

target_link_libraries(simple2d-entry PRIVATE simple2d-engine)

## EXECUTABLE
set(GAME_DIRECTORY /Users/maxortner/Projects/censor)

add_executable(main ${GAME_DIRECTORY}/src/main.cpp)

target_compile_definitions(main PUBLIC -DSOURCE_DIR="${GAME_DIRECTORY}/out")
target_link_libraries(main PUBLIC simple2d-engine simple2d-graphics simple2d-lua simple2d-entry flecs)