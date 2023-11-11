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
    struct TypeMap
    {
        constexpr static int LuaType = LUA_TUSERDATA;

        inline static bool 
        check(lua_State* L)
        {
            return lua_isuserdata(L, -1);
        }

        inline static void
        push(lua_State* L, const T& val)
        {
            auto* ptr = lua_newuserdata(L, sizeof(T));
            *reinterpret_cast<T*>(ptr) = val;
        }

        inline static T
        construct(lua_State* L)
        {
            return *reinterpret_cast<T*>(lua_touserdata(L, -1));
        }
    };
    template<>
    struct TypeMap<Lua::Number>
    {
        constexpr static int LuaType = LUA_TNUMBER;

        inline static bool 
        check(lua_State* L)
        {
            return lua_isnumber(L, -1);
        }

        inline static void
        push(lua_State* L, Lua::Number number)
        {
            lua_pushnumber(L, number);
        }

        inline static Lua::Number 
        construct(lua_State* L)
        {
            return static_cast<Lua::Number>(lua_tonumber(L, -1));
        }
    };

    template<>
    struct TypeMap<Lua::String>
    {
        constexpr static int LuaType = LUA_TSTRING;

        inline static bool 
        check(lua_State* L)
        {
            return lua_isstring(L, -1);
        }

        inline static void
        push(lua_State* L, Lua::String string)
        {
            lua_pushstring(L, string.c_str());
        }

        inline static Lua::String 
        construct(lua_State* L)
        {
            return Lua::String(lua_tostring(L, -1));
        }
    };
    
    template<>
    struct TypeMap<Lua::Function>
    {
        const static int LuaType = LUA_TFUNCTION;

        inline static bool
        check(lua_State* L)
        {
            return lua_iscfunction(L, -1);
        }

        inline static void
        push(lua_State* L, Lua::Function function)
        {
            lua_pushcfunction(L, function);
        }

        inline static Lua::Function
        construct(lua_State* L)
        {
            return lua_tocfunction(L, -1);
        }
    };

    template<>
    struct TypeMap<Lua::Boolean>
    {
        constexpr static int LuaType = LUA_TBOOLEAN;

        inline static bool 
        check(lua_State* L)
        {
            return lua_isboolean(L, -1);
        }

        inline static void
        push(lua_State* L, Lua::Boolean boolean)
        {
            lua_pushboolean(L, boolean);
        }

        inline static Lua::Boolean 
        construct(lua_State* L)
        {
            return lua_toboolean(L, -1);
        }
    };

    template<>
    struct TypeMap<Lua::Table>
    {
        constexpr static int LuaType = LUA_TTABLE;

        inline static void
        push(lua_State* L, const Table& val)
        {
            val.toStack(L);
        }

        inline static Lua::Table
        construct(lua_State* L)
        {
            return Lua::Table(L);
        }
        
        inline static bool
        check(lua_State* L)
        {
            return lua_istable(L, -1);
        }
    };

} // CompileTime

} // S2D::Lua