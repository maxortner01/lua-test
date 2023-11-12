/**
 * @file Engine.hpp
 * @author Max Ortner (contact@maxortner.com)
 * @brief This is the 2D engine portion of the code.
 * 
 * As of right now, it's a little finicky, so pay close attention.
 * 
 * This file should only be included after the user has defined two macros:
 * COMPONENT_ENUM_NAME and COMPONENT_STRUCT_NAME. The first is defined as the name
 * of an enum class that requires one member Count and all others are the names of
 * the components the user defines.
 * 
 * The second is the name of the struct that contains three things, first is a struct
 * called Data which is the actual data structure used as the component, second and third
 * are toTable(...) and fromTable(...) which specify how a void* goes to or from a Lua::Table.
 * 
 * These defines *and* the respective enum and struct definitions should come *before* you inclue
 * this file. It should work great if this is all done correctly.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "Engine/Library.hpp"