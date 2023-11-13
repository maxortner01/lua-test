#pragma once

#include "TypeMap.hpp"

#include "../Util.hpp"

namespace S2D::Lua
{
    struct Runtime;
}

namespace S2D::Lua::Lib
{
    /**
     * @brief Represents a base library package in a Lua script
     */
    struct Base : Util::NoCopy
    {
        using Map = std::unordered_map<std::string, Lua::Function>;

        /**
         * @brief Helper function extracting a list of types arguments from a Lua stack.
         * @tparam Args Types of the arguments
         * @param L     Lua state
         * @return std::tuple<Args...> Values of the Lua arguments
         */
        template<typename... Args>
        static std::tuple<Args...>
        extractArgs(State L);

        /**
         * @brief Registers the functions in this library with the given runtime.
         * @param runtime Lua runtime to register the functions with
         */
        void registerFunctions(Lua::Runtime& runtime) const;

        Lua::Table asTable() const;

    protected:
        Base(
            const std::string& name,
            const Map& funcs);
        
        ~Base() = default;

        std::string _name;
        Map         _funcs;
    };

    /* struct Base */
    template<typename... Args>
    std::tuple<Args...>
    Base::extractArgs(State L)
    {
        assert(lua_gettop(L) == sizeof...(Args));
        std::tuple<Args...> values;
        Util::CompileTime::static_for<sizeof...(Args)>([&](auto n) {
            if (lua_isnil(L, -1)) assert(false);

            const std::size_t I = n;
            const auto i = sizeof...(Args) - I - 1;
            using Type = Util::CompileTime::NthType<i, Args...>;
            assert(Lua::CompileTime::TypeMap<Type>::check(L));
            
            const auto count = lua_gettop(L);
            std::get<i>(values) = Lua::CompileTime::TypeMap<Type>::construct(L);
            if (lua_gettop(L) == count) lua_pop(L, 1);
        });
        return values;
    }

} // S2D::Lua::Lib