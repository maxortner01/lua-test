#include <iostream>
#include <cassert>
#include <optional>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#ifndef SOURCE_DIR
#define SOURCE_DIR ""
#endif

bool lua_check(lua_State* L, int r, std::optional<int> line = std::nullopt)
{
    if (r != LUA_OK)
    {
        std::string error_message = lua_tostring(L, -1);
        std::cout << "Message ";
        if (line.has_value()) std::cout << "from line " << line.value();
        std::cout << ": " << error_message << "\n";
        return false;
    }
    return true;
}

namespace Lua
{
    using String = std::string;
    using Number = float;

namespace CompileTime
{
    // https://stackoverflow.com/questions/54268425/enumerating-over-a-fold-expression
    template<std::size_t... inds, class F>
    constexpr void static_for_impl(std::index_sequence<inds...>, F&& f)
    {
        (f(std::integral_constant<std::size_t, inds>{}), ...);
    }

    template<std::size_t N, class F>
    constexpr void static_for(F&& f)
    {
        static_for_impl(std::make_index_sequence<N>{}, std::forward<F>(f));
    }

    template<int N, typename... Ts>
    using NthType = typename std::tuple_element<N, std::tuple<Ts...>>::type;

    template<typename T>
    struct TypeMap;

    template<>
    struct TypeMap<Lua::Number>
    {
        inline static bool 
        check(lua_State* L)
        {
            return lua_isnumber(L, -1);
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
        inline static bool 
        check(lua_State* L)
        {
            return lua_isstring(L, -1);
        }

        inline static Lua::String 
        construct(lua_State* L)
        {
            return Lua::String(lua_tostring(L, -1));
        }
    };
};

    template<typename T = int>
    struct Error
    {
        Error(T num) :
            _num(num)
        {   }

        T code() const { return _num; }
    
    private:
        T _num;
    };

    template<typename T, typename E = int>
    struct Result
    {
        Result(T&& val) :
            _val(std::move(val))
        {   }

        Result(Error<E> err) :
            _err(err)
        {   }

        Result(const Result&) = delete;
        ~Result() = default;

        T&       value()       { assert(good());  return _val.value(); }
        Error<E> error() const { assert(!good()); return _err.value(); }
        bool     good()  const { return _val.has_value(); }

        operator bool() const { return good(); }

    private:
        std::optional<T> _val;
        std::optional<Error<E>> _err;
    };

    template<typename E>
    struct Result<void, E>
    {
        Result() = default;

        Result(Error<E> err) :
            _err(err)
        {   }

        Result(const Result&) = delete;
        ~Result() = default;

        Error<E> error() const { assert(!good()); return _err.value(); }
        bool     good()  const { return !_err.has_value(); }

        operator bool() const { return good(); }

    private:
        std::optional<Error<E>> _err;
    };

    struct Table
    {
        friend class Runtime;

        template<typename T>
        T& get(const std::string& name)
        {
            assert(dictionary.count(name));
            return *static_cast<T*>(dictionary.at(name).get());
        }

        ~Table() = default;

    private:
        std::unordered_map<std::string, std::shared_ptr<void>> dictionary;
    };

    struct Runtime
    {
        enum class ErrorCode
        {
            None,
            NotNumber,
            NotString,
            NotFunction,
            WrongTypeTable,
            FunctionError,
            TypeMismatch,
            VariableDoesntExist
        };

        template<typename T>
        using RuntimeResult = Result<T, ErrorCode>;

        Runtime(const std::string& filename) :
            L(luaL_newstate()),
            _good(lua_check(L, luaL_dofile(L, filename.c_str())))
        {   
            luaL_openlibs(L);
        }

        Runtime(Runtime&& r) :
            L(r.L),
            _good(r._good)
        {
            r.L = nullptr;
        }

        Runtime(const Runtime&) = delete;

        ~Runtime()
        {
            if (L)
            {
                lua_close(L);
                L = nullptr;
            }
        }

        RuntimeResult<void> 
        registerFunction(
            const std::string& table_name,
            const std::string& func_name,
            lua_CFunction func)
        {
            lua_getglobal(L, table_name.c_str());
            if (!lua_istable(L, -1))
            {
                lua_createtable(L, 0, 1);
                lua_setglobal(L, table_name.c_str());
                lua_getglobal(L, table_name.c_str());
                if (!lua_istable(L, -1)) return { ErrorCode::VariableDoesntExist };
            }
            lua_pushstring(L, func_name.c_str());
            lua_pushcfunction(L, func);
            lua_settable(L, -3);

            lua_pop(L, 1);

            return { };
        }

        template<typename T>
        RuntimeResult<T>
        get(const std::string& name)
        {
            lua_getglobal(L, name.c_str());
            if (!CompileTime::TypeMap<T>::check(L)) return { ErrorCode::TypeMismatch };
            return { CompileTime::TypeMap<T>::construct(L) };
        }

        template<>
        RuntimeResult<Table>
        get(const std::string& name)
        {
            lua_getglobal(L, name.c_str());
            if (!lua_istable(L, -1)) { lua_pop(L, 1); return { ErrorCode::WrongTypeTable }; }

            Table table;

            lua_pushnil(L);
            while (lua_next(L, -2) != 0)
            {
                assert(lua_type(L, -2) == LUA_TSTRING);
                
                std::string key = lua_tostring(L, -2);
                std::shared_ptr<void> value;

                const auto emplace = [](auto value) {
                    std::shared_ptr<void> ptr = std::shared_ptr<void>(
                        std::malloc(sizeof(value)),
                        [](void* ptr) { std::free(ptr); }
                    );
                    *static_cast<decltype(value)*>(ptr.get()) = value;
                    return ptr;
                };

                switch(lua_type(L, -1))
                {
                case LUA_TNUMBER: value = emplace(static_cast<Lua::Number>(lua_tonumber(L, -1))); break;
                case LUA_TSTRING: value = emplace(Lua::String(lua_tostring(L, -1)));              break;
                }
                
                table.dictionary.insert(std::pair(key, value));
                lua_pop(L, 1);
            }
            lua_pop(L, 1);

            return { std::move(table) };
        }

        template<typename... Return, typename... Args>
        RuntimeResult<std::tuple<Return...>>
        runFunction(
            const std::string& name,
            Args&&... args)
        {
            lua_getglobal(L, name.c_str());
            if (!lua_isfunction(L, -1)) { lua_pop(L, 1); return { ErrorCode::NotFunction }; }

            auto args_set = std::tuple(std::forward<Args>(args)...);
            CompileTime::static_for<sizeof...(args)>([&](auto n){
                constexpr std::size_t I = n;
                using Type = CompileTime::NthType<I, Args...>;
                lua_pushnumber(L, std::get<I>(args_set));
            });

            if (!lua_check(L, lua_pcall(L, sizeof...(Args), sizeof...(Return), 0))) return { ErrorCode::FunctionError };

            bool err = false;
            auto left = sizeof...(Return);
            auto return_vals = std::tuple<Return...>();
            CompileTime::static_for<sizeof...(Return)>([&](auto n) {
                constexpr std::size_t I = n;
                using Type = CompileTime::NthType<I, Return...>;
                using Map  = CompileTime::TypeMap<Type>;

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

            assert(!left);
            if (err)
            {
                lua_pop(L, left);
                return { ErrorCode::TypeMismatch };
            }

            return { std::move(return_vals) };
        }

        bool good() const { return _good; }

        operator bool() const { return good(); }

    private:
        lua_State* L;
        bool _good;
    };
}

template<typename T>
struct Singleton
{
    static T& get()
    {
        if (!instance) instance = new T();
        return *instance;
    }

    static void destroy()
    {
        if (instance) delete instance;
        instance = nullptr;
    }

private:
    inline static T* instance = nullptr;
};

namespace LuaLib
{
    struct Library
    {
        void registerFunctions(Lua::Runtime& runtime)
        {
            for (auto& p : _funcs)
                runtime.registerFunction(_name, p.first, p.second);
        }

    protected:
        Library(
            const std::string& name,
            const std::unordered_map<std::string, lua_CFunction>& funcs) :
                _name(name),
                _funcs(funcs)
        {   }

        std::string _name;
        std::unordered_map<std::string, lua_CFunction> _funcs;
    };

    struct Debug : Library, Singleton<Debug>
    {
        static int log(lua_State* L)
        {
            assert(lua_gettop(L) == 1);
            assert(lua_isstring(L, -1));

            auto val = std::string(lua_tostring(L, -1));

            spdlog::get("lua")->info(val);

            return 0;
        }

        static int _assert(lua_State* L)
        {
            assert(lua_gettop(L) == 1);
            assert(lua_isboolean(L, -1));

            lua_Debug debug;
            lua_getstack(L, 1, &debug);
            lua_getinfo(L, "nSl", &debug);

            bool expr = lua_toboolean(L, -1);
            if (!expr)
            {
                std::filesystem::path path(debug.source);
                spdlog::get("lua")->critical("Assertion failed: file {}, file {}.", path.filename().c_str(), debug.currentline);
                exit(1);
            }
            return 0;
        }

    private:
        friend class Singleton<Debug>;

        Debug() : Library("Debug",
            {
                { "log",    Debug::log     },
                { "assert", Debug::_assert }
            })
        {   }
    };
}

namespace Engine
{
    static Lua::Runtime createRuntime(const std::string& filename)
    {
        Lua::Runtime runtime(filename);
        assert(runtime);

        using namespace LuaLib;
        Debug::get().registerFunctions(runtime);
        Debug::destroy();

        return runtime;
    }
}

int main()
{
    auto runtime = Engine::createRuntime(SOURCE_DIR "/scripts/test.lua");

    auto c_log   = spdlog::stdout_color_mt("main");
    auto lua_log = spdlog::stdout_color_mt("lua");

    c_log->trace("Starting Engine");
    runtime.runFunction<>("Start");
    
    const auto [name, level] = [&]()
    {
        auto table_res = runtime.get<Lua::Table>("player");
        if (!table_res) { std::cout << "Table failed: " << (int)table_res.error().code() << "\n"; assert(false); }
        auto& table = table_res.value();
        return std::pair(table.get<Lua::String>("Name"), table.get<Lua::Number>("Level"));
    }();

    c_log->info("Player: {} [Lvl: {}]", name, level);
    
    c_log->trace("Destroying Engine");
    return 0;
}
