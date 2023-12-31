#pragma once

#include "TypeMap.hpp"

#include "../Def.hpp"
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

    namespace detail
    {
    
    std::size_t __getTop(State L);
    bool        __isNil(State L);
    void        __pop(State L, uint32_t n);

    }

    /* struct Base */
    template<typename... Args>
    std::tuple<Args...>
    Base::extractArgs(State L)
    {
        S2D_ASSERT(detail::__getTop(L) == sizeof...(Args), "Lua arguments do not match expected args.");
        std::tuple<Args...> values;
        Util::CompileTime::static_for<sizeof...(Args)>([&](auto n) {
            S2D_ASSERT(!detail::__isNil(L), "Argument nil");

            const std::size_t I = n;
            const auto i = sizeof...(Args) - I - 1;
            using Type = Util::CompileTime::NthType<i, Args...>;
            S2D_ASSERT(Lua::CompileTime::TypeMap<Type>::check(L), "Type mismatch");
            
            const auto count = detail::__getTop(L);
            std::get<i>(values) = Lua::CompileTime::TypeMap<Type>::construct(L);
            if (detail::__getTop(L) == count) detail::__pop(L, 1);
        });
        return values;
    }

} // S2D::Lua::Lib
