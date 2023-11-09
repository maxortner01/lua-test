#include <iostream>
#include <cassert>
#include <optional>
#include <unordered_map>
#include <memory>

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

    struct Table
    {
        friend class File;

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

    struct File
    {
        enum class ErrorCode
        {
            None,
            NotNumber,
            NotString,
            NotFunction,
            WrongTypeTable,
            FunctionError,
            TypeMismatch
        };

        template<typename T>
        using FileResult = Result<T, ErrorCode>;

        File(const std::string& filename) :
            L(luaL_newstate()),
            _good(lua_check(L, luaL_dofile(L, filename.c_str())))
        {   
            luaL_openlibs(L);
        }

        ~File()
        {
            if (L)
            {
                lua_close(L);
                L = nullptr;
            }
        }

        FileResult<Table>
        getTable(const std::string& name)
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

        FileResult<Lua::Number> 
        getNumber(const std::string& name)
        {
            lua_getglobal(L, name.c_str());
            if (!lua_isnumber(L, -1)) return { ErrorCode::NotNumber };
            return { static_cast<Lua::Number>(lua_tonumber(L, -1)) };
        }

        template<typename... Return, typename... Args>
        FileResult<std::tuple<Return...>>
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

int main()
{
    Lua::File file(SOURCE_DIR "/scripts/test.lua");
    assert(file);

    file.runFunction<>("Start");
    
    const auto [name, level] = [&]()
    {
        auto table_res = file.getTable("player");
        if (!table_res) { std::cout << "Table failed: " << (int)table_res.error().code() << "\n"; assert(false); }
        auto& table = table_res.value();
        return std::pair(table.get<Lua::String>("Name"), table.get<Lua::Number>("Level"));
    }();

    std::cout << "Player: " << name << " [Lvl: " << level << "]\n";
    
    while (true)
    {
        file.runFunction<>("Update");
    }

    /*
    auto res = file.runFunction<Lua::Number>("AddStuff", 2.f, 4.f);
    assert(res);
    std::cout << std::get<0>(res.value()) << "\n";*/

    //auto res = file.getNumberFromFunction("AddStuff", 2.f, 4.f);
    //assert(res);
    //std::cout << "2 + 4 = " << res.value() << "\n";
}
