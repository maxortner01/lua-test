cmake_minimum_required(VERSION 3.8)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH OFF)

find_package(Lua REQUIRED)

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

set(GRAPHICS_INCLUDES ${CMAKE_SOURCE_DIR}/extern/freetype/include)

set(GRAPHICS_LIBRARIES SDL3::SDL3 libglew_static freetype)

add_library(simple2d-graphics SHARED ${GRAPHICS_SOURCES})

target_include_directories(simple2d-graphics PRIVATE ${GRAPHICS_INCLUDES})
target_link_libraries(simple2d-graphics PRIVATE ${GRAPHICS_LIBRARIES})

# ENGINE
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/spdlog)

set(FLECS_STATIC OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/flecs)

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
    ${CMAKE_SOURCE_DIR}/src/Engine/Core/Render.cpp

    ${CMAKE_SOURCE_DIR}/src/Engine/Components.cpp
    ${CMAKE_SOURCE_DIR}/src/Log/Library.cpp
    ${CMAKE_SOURCE_DIR}/src/Log/Log.cpp)

add_library(simple2d-engine SHARED ${ENGINE_SOURCES} ${GRAPHICS_SOURCES})

target_link_libraries(simple2d-engine PRIVATE simple2d-lua ${GRAPHICS_LIBRARIES} spdlog::spdlog flecs ${LUA_LIBRARIES})
target_include_directories(simple2d-engine PRIVATE ${LUA_INCLUDE_DIR} ${GRAPHICS_INCLUDES})

## APPLICATION
set(APPLICATION_SOURCES ${CMAKE_SOURCE_DIR}/src/Engine/Application.cpp)

add_library(simple2d-entry STATIC ${APPLICATION_SOURCES})

target_link_libraries(simple2d-entry PRIVATE simple2d-engine)

## EXECUTABLE
set(GAME_DIRECTORY /Users/maxortner/Projects/censor)

add_executable(main ${GAME_DIRECTORY}/src/main.cpp)

target_compile_definitions(main PUBLIC -DSOURCE_DIR="${GAME_DIRECTORY}/out")
target_link_libraries(main PUBLIC simple2d-engine simple2d-lua simple2d-entry flecs)