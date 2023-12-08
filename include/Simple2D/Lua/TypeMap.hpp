#pragma once

#include "Lua.hpp"
#include "Table.hpp"

namespace S2D::Lua
{
    /* Types */

    using String = std::string;
    using Number = float;
    using Boolean = bool;
    using Function = lua_CFunction;

namespace CompileTime
{
    template<typename T>
    struct TypeMap;

    template<>
    struct TypeMap<void*>
    {
        constexpr static int LuaType = LUA_TUSERDATA;

        inline static bool 
        check(State L)
        {
            return lua_isuserdata(L, -1);
        }

        inline static void
        push(State L, void* const & val)
        {
            auto* ptr = lua_newuserdata(L, sizeof(void*));
            *reinterpret_cast<void**>(ptr) = val;
        }

        inline static void*
        construct(State L)
        {
            return *reinterpret_cast<void**>(lua_touserdata(L, -1));
        }
    };
    template<>
    struct TypeMap<Lua::Number>
    {
        constexpr static int LuaType = LUA_TNUMBER;

        inline static bool 
        check(State L)
        {
            return lua_isnumber(L, -1);
        }

        inline static void
        push(State L, Lua::Number number)
        {
            lua_pushnumber(L, number);
        }

        inline static Lua::Number 
        construct(State L)
        {
            return static_cast<Lua::Number>(lua_tonumber(L, -1));
        }
    };

    template<>
    struct TypeMap<Lua::String>
    {
        constexpr static int LuaType = LUA_TSTRING;

        inline static bool 
        check(State L)
        {
            return lua_isstring(L, -1);
        }

        inline static void
        push(State L, Lua::String string)
        {
            lua_pushstring(L, string.c_str());
        }

        inline static Lua::String 
        construct(State L)
        {
            return Lua::String(lua_tostring(L, -1));
        }
    };
    
    template<>
    struct TypeMap<Lua::Function>
    {
        const static int LuaType = LUA_TFUNCTION;

        inline static bool
        check(State L)
        {
            return lua_iscfunction(L, -1);
        }

        inline static void
        push(State L, Lua::Function function)
        {
            lua_pushcfunction(L, function);
        }

        inline static Lua::Function
        construct(State L)
        {
            return lua_tocfunction(L, -1);
        }
    };

    template<>
    struct TypeMap<Lua::Boolean>
    {
        constexpr static int LuaType = LUA_TBOOLEAN;

        inline static bool 
        check(State L)
        {
            return lua_isboolean(L, -1);
        }

        inline static void
        push(State L, Lua::Boolean boolean)
        {
            lua_pushboolean(L, boolean);
        }

        inline static Lua::Boolean 
        construct(State L)
        {
            return lua_toboolean(L, -1);
        }
    };

    template<>
    struct TypeMap<Lua::Table>
    {
        constexpr static int LuaType = LUA_TTABLE;

        inline static void
        push(State L, const Table& val)
        {
            val.toStack(L);
        }

        inline static Lua::Table
        construct(State L)
        {
            return Lua::Table(L);
        }
        
        inline static bool
        check(State L)
        {
            return lua_istable(L, -1);
        }
    };

} // CompileTime

} // S2D::Lua