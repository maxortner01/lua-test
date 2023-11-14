#pragma once

#include "../Lua.hpp"

namespace S2D::Engine
{
#ifdef DOCS

/**
 * @brief This is documentation for the scripting API for Lua.
 */
namespace LuaAPI
{
    /**
     * @brief Represents the entity that a script is attached to
     * 
     */
    struct Entity
    {
        /**
         * @brief Get a component of a specific type from the entity
         * 
         * @param type The type of component
         * @return Component The component associated with that type
         */
        Component getComponent(ComponentType type);
        void setComponent(Component)
    };
}

#endif

    /**
     * @brief Represents the methods associated with manipulating an entity in Lua.
     */
    struct Entity : Lua::Lib::Base
    {
        static int getComponent(Lua::State L);
        static int setComponent(Lua::State L);
        static int destroy(Lua::State L);
        static int addScript(Lua::State L);

        Entity();
    };
}