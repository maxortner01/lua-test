#pragma once

#include <iostream>

#include "../Util.hpp"

#include "Lib.hpp"
#include "TypeMap.hpp"

namespace S2D::Lua
{

    /**
     * @brief Represents a single Lua runtime.
     */
    struct Runtime : Util::NoCopy
    {
        enum class ErrorCode
        {
            None,
            TypeMismatch,
            VariableDoesntExist,
            NotFunction,
            FunctionError
        };

        template<typename T>
        using Result = Util::Result<T, Util::Error<ErrorCode>>;

        /**
         * @brief Construct a Lua runtime from a script
         * @param filename File path to the script
         */
        Runtime(const std::string& filename);
        
        Runtime(Runtime&& r);
        ~Runtime();

        /**
         * @brief Creates a runtime with the given Libraries loaded into it.
         * @tparam Libraries List of library types that are derived from \ref Lua::Lib::Base.
         * @param filename Name of the file to load into the runtime
         * @return Runtime The created runtime 
         */
        template<typename... Libraries>
        static Runtime create(const std::string& filename);

        /**
         * @brief Registers a C++ function for use in the Lua runtime
         * @param table_name Name of the table to register function in
         * @param func_name Name of the function to call in Lua
         * @param function Pointer to a static function 
         * @return Result<void> Returns if an error has occured
         */
        Result<void>
        registerFunction(
            const std::string& table_name,
            const std::string& func_name,
            Lua::Function function);

        /**
         * @brief Get a global variable by name from runtime
         * @tparam T Type of the global variable (supported types in Lua namespace)
         * @param name Name of the variable in the script
         * @return Result<T> The value of the variable or error
         */
        template<typename T>
        Result<T>
        getGlobal(const std::string& name);

        /**
         * @brief Set a global variable from a value to a name
         * @tparam T Type of the global variable (supported types in Lua namespace)
         * @param name  Name of the global variable
         * @param value Value of the global variable
         * @return Result<void> The status of the operation
         */
        template<typename T>
        Result<void>
        setGlobal(const std::string& name, const T& value);

        /**
         * @brief Invokes a Lua function from this environment
         * @tparam Return Expected return types from the function 
         * @tparam Args   Arguments to pass into the function
         * @param name Name of the function
         * @param args Values of the arguments
         * @return Result<std::tuple<Return...>> Contains the values returned from the function or error
         */
        template<typename... Return, typename... Args>
        Result<std::tuple<Return...>>
        runFunction(
            const std::string& name,
            Args&&... args);

        bool     good() const;
        operator bool() const;

    private:
        State L;
        bool _good;
    };

    template<typename... Libraries>
    Runtime Runtime::create(const std::string& filename)
    {
        using namespace Util::CompileTime;

        Runtime runtime(filename);

        static_for<sizeof...(Libraries)>([&](auto n) 
        {
            const std::size_t i = n;
            using Library = NthType<i, Libraries...>;
            static_assert(std::is_base_of_v<Lua::Lib::Base, Library>);
            Library().registerFunctions(runtime);
        });

        return runtime;
    }

    template<typename... Return, typename... Args>
    Runtime::Result<std::tuple<Return...>>
    Runtime::runFunction(
        const std::string& name,
        Args&&... args)
    {
        lua_getglobal(L, name.c_str());
        if (!lua_isfunction(L, -1)) { lua_pop(L, 1); return { Runtime::ErrorCode::NotFunction }; }

        auto args_set = std::tuple(std::forward<Args>(args)...);
        Util::CompileTime::static_for<sizeof...(args)>([&](auto n){
            constexpr std::size_t I = n;
            using Type = std::remove_reference_t<Util::CompileTime::NthType<I, Args...>>;
            CompileTime::TypeMap<Type>::push(L, std::get<I>(args_set));
        });

        if (lua_pcall(L, sizeof...(Args), sizeof...(Return), 0) != LUA_OK) return { { ErrorCode::FunctionError, std::string(lua_tostring(L, -1)) } };
        
        bool err = false;
        auto left = sizeof...(Return);
        auto return_vals = std::tuple<Return...>();
        Util::CompileTime::static_for<sizeof...(Return)>([&](auto n) {
            constexpr std::size_t I = n;
            using Type = Util::CompileTime::NthType<I, Return...>;
            using Map  = Lua::CompileTime::TypeMap<Type>;

            if (!err)
            {
                if (Map::check(L))
                {
                    std::get<I>(return_vals) = Map::construct(L);
                    left--;
                }
                else err = true;
            }
        });

        S2D_ASSERT(!left, "Still arguments left.");
        if (err)
        {
            lua_pop(L, (int)left);
            return { ErrorCode::TypeMismatch };
        }

        return { std::move(return_vals) };
    }

} // S2D::Lua
