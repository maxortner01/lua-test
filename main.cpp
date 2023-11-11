#include <iostream>
#include <chrono>
#include <cassert>
#include <optional>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>

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
    using Boolean = bool;
    using Function = lua_CFunction;

    struct Table;

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
            //lua_pushboolean(L, boolean);
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
        const T& value() const { assert(good());  return _val.value(); }
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
        struct Data
        {
            std::shared_ptr<void> data;
            int type;

            template<typename T>
            static Data fromValue(const T& value)
            {
                return Data {
                    .data = emplace(value),
                    .type = CompileTime::TypeMap<T>::LuaType
                };
            }

            template<typename T>
            static std::shared_ptr<void> emplace(const T& value)
            {
                std::shared_ptr<void> ptr = std::shared_ptr<void>(
                    std::malloc(sizeof(T)),
                    [](void* ptr) { std::free(ptr); }
                );
                *static_cast<T*>(ptr.get()) = value;
                return ptr;
            }
        };

        using Map = std::unordered_map<std::string, Data>;

        template<typename T>
        T& get(const std::string& name)
        {
            assert(dictionary.count(name));
            return *static_cast<T*>(dictionary.at(name).data.get());
        }

        template<typename T>
        const T& get(const std::string& name) const
        {
            assert(dictionary.count(name));
            return *static_cast<T*>(dictionary.at(name).data.get());
        }

        const Map& getMap() const { return dictionary; }

        template<typename T>
        void set(const std::string& name, const T& value)
        {
            dictionary.insert(std::pair(name, Data::fromValue(value)));
        }

        void toStack(lua_State* L) const
        {
            using namespace CompileTime;

            lua_newtable(L);

            for (const auto& p : dictionary)
            {
                lua_pushstring(L, p.first.c_str());

                switch (p.second.type)
                {
                case LUA_TNUMBER:  TypeMap<Lua::Number> ::push(L, get<Lua::Number> (p.first)); break;
                case LUA_TSTRING:  TypeMap<Lua::String> ::push(L, get<Lua::String> (p.first)); break;
                case LUA_TBOOLEAN: TypeMap<Lua::Boolean>::push(L, get<Lua::Boolean>(p.first)); break;
                case LUA_TTABLE:
                {
                    const auto& table = get<Lua::Table>(p.first);
                    table.toStack(L);
                    break;
                }
                case LUA_TFUNCTION: TypeMap<Lua::Function>::push(L, get<Lua::Function>(p.first)); break;
                default: TypeMap<uint64_t>::push(L, get<uint64_t>(p.first)); break; // need to make this general
                }

                lua_settable(L, -3);
            }
        }

        Table() = default;

        Table(const Map& map) :
            dictionary(map)
        {   }
        
        void
        fromTable(const Table& table)
        {
            dictionary = table.dictionary;
        }

        void 
        superImpose(const Map& map)
        {
            for (const auto& p : map)
                dictionary.insert(p);
        }

        // Constructs the table from the current stack leaves the stack without the table
        // Assumes the table is at index -1
        Table(lua_State* L)
        {
            lua_pushnil(L);
            while (lua_next(L, -2) != 0)
            {
                assert(lua_type(L, -2) == LUA_TSTRING);
                
                std::string key = lua_tostring(L, -2);
                std::shared_ptr<void> value;

                const auto count = lua_gettop(L);
                const auto type = lua_type(L, -1);
                switch(type)
                {
                case LUA_TNUMBER:   value = Data::emplace(static_cast<Lua::Number>(lua_tonumber(L, -1))); break;
                case LUA_TSTRING:   value = Data::emplace(Lua::String(lua_tostring(L, -1)));              break;
                case LUA_TBOOLEAN:  value = Data::emplace(lua_toboolean(L, -1));                          break;
                case LUA_TUSERDATA: value = Data::emplace(lua_touserdata(L, -1));                         break;
                case LUA_TTABLE:    value = Data::emplace(Table(L)); break;
                }
                
                dictionary.insert(std::pair(key, Data{ .data = value, .type = type }));
                if (count == lua_gettop(L)) lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }

        ~Table() = default;

    private:
        Map dictionary;
    };

namespace CompileTime
{
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
}

    struct TableStruct
    {
        Table toTable()
        {
            return Table(get_fields());
        }

    protected:
        virtual Table::Map get_fields() const = 0;
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
            return { Table(L) };
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
                using Type = std::remove_reference_t<CompileTime::NthType<I, Args...>>;
                CompileTime::TypeMap<Type>::push(L, std::get<I>(args_set));
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
    struct TestStruct : Lua::TableStruct
    {
        float coolness;

    protected:
        Lua::Table::Map get_fields() const override
        {
            return {
                { "coolness", Lua::Table::Data::fromValue(coolness) }
            };
        }
    };

    struct Library
    {
        template<typename... Args>
        static std::tuple<Args...>
        extractArgs(lua_State* L)
        {
            assert(lua_gettop(L) == sizeof...(Args));
            std::tuple<Args...> values;
            Lua::CompileTime::static_for<sizeof...(Args)>([&](auto n) {
                const std::size_t I = n;
                const auto i = sizeof...(Args) - I - 1;
                using Type = Lua::CompileTime::NthType<i, Args...>;
                assert(Lua::CompileTime::TypeMap<Type>::check(L));
                
                const auto count = lua_gettop(L);
                std::get<i>(values) = Lua::CompileTime::TypeMap<Type>::construct(L);
                if (lua_gettop(L) == count) lua_pop(L, 1);
            });
            return values;
        }
        
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

    struct Debug : Library
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
                const auto path = [&]() -> auto
                {
                    auto string = std::string(debug.source);
                    string.erase(string.begin());
                    return std::filesystem::path(string);
                }();

                // Read in the line from the file so we can see what the expression is
                // Only works if line is of format Debug.Assert( ... ), which should be true
                // since assert returns void
                const auto line = [&]() {
                    std::ifstream file(path);
                    assert(file);

                    std::string assert_line;
                    uint32_t lineno = 0;
                    for (std::string line; std::getline(file, line);)
                    {
                        if (++lineno == debug.currentline) { assert_line = line; break; }
                    }
                    auto first = assert_line.find_first_of('(');
                    auto last = assert_line.find_last_of(')');
                    return assert_line.substr(first + 1, last - first - 1);
                }();

                auto logger = spdlog::get("lua");
                assert(logger);
                logger->critical("Assertion ({}) failed in file {} on line {}.", line, path.filename().c_str(), debug.currentline);
                exit(1);
            }

            return 0;
        }

        static int getTable(lua_State* L)
        {
            assert(lua_gettop(L) == 0);

            TestStruct test;
            test.coolness = 42.f;

            auto table = test.toTable();
            table.set("good", true);
            table.toStack(L);

            return 1;
        }

        static int distance(lua_State* L)
        {
            const auto count = lua_gettop(L);

            float val = 0.f;
            if (count == 1)
            {
                assert(lua_istable(L, -1));
                Lua::Table input(L);

                for (const auto& p : input.getMap())
                    val += pow(input.get<Lua::Number>(p.first), 2);
            }
            else if (count > 1)
            {
                for (uint32_t i = 0; i < count; i++)
                {
                    assert(lua_isnumber(L, -1));
                    auto number = static_cast<float>(lua_tonumber(L, -1));
                    val += pow(number, 2);
                    lua_pop(L, 1);
                }
            }

            assert(!lua_gettop(L));
            lua_pushnumber(L, sqrt(val));
            return 1;
        }

        Debug() : Library("Debug",
            {
                { "log",      Debug::log      },
                { "assert",   Debug::_assert  },
                { "getTable", Debug::getTable },
                { "distance", Debug::distance }
            })
        {   }
    };
}

#include <flecs.h>

enum class Component
{
    Position,
    RigidBody,
    Count
};

const char* operator*(Component c)
{
    switch(c)
    {
    case Component::Position:  return "Position";
    case Component::RigidBody: return "RigidBody";
    default: return "";
    }
}

template<Component C>
struct Comp;

template<>
struct Comp<Component::Position>
{
    static constexpr Component Type = Component::Position;

    struct Data
    {
        float x, y;
    };

    static Lua::Table getTable(const Data& data)
    {
        Lua::Table table;
        table.set("x", data.x);
        table.set("y", data.y);
        return table;
    }

    static void fromTable(const Lua::Table& table, void* _data)
    {
        auto* data = reinterpret_cast<Data*>(_data);
        data->x = table.get<float>("x");
        data->y = table.get<float>("y");
    }
};

template<>
struct Comp<Component::RigidBody>
{
    static constexpr Component Type = Component::RigidBody;

    struct Data
    {
        struct {
            float x, y;
        } velocity;
    };

    static Lua::Table getTable(const Data& data)
    {
        Lua::Table table;
        Lua::Table velocity;
        velocity.set("x", data.velocity.x);
        velocity.set("y", data.velocity.y);
        table.set("velocity", velocity);
        return table;
    }

    static void fromTable(const Lua::Table& table, void* _data)
    {
        auto* data = reinterpret_cast<Data*>(_data);
        const auto& velocity = table.get<Lua::Table>("velocity");
        data->velocity.x = velocity.get<float>("x");
        data->velocity.y = velocity.get<float>("y");
    }
};

bool 
tableToComponent(const Lua::Table& table, void* data)
{
    bool found = false;

    Lua::CompileTime::static_for<(int)Component::Count>([&](auto n)
    {
        if (found) return;

        constexpr std::size_t i = n;
        constexpr Component component = (Component)i;
        const auto comp_name = std::string(*component);
        const auto& name = table.get<Lua::String>("name");

        if (name == comp_name)
        {
            found = true;
            Comp<component>::fromTable(table, data);
        }
    });

    if (!found) return false;

    return true;
}

Lua::Table
componentToTable(const std::string& name, const void* data)
{
    Lua::Table table;
    bool found = false;

    Lua::CompileTime::static_for<(int)Component::Count>([&](auto n)
    {
        if (found) return;

        constexpr std::size_t i = n;
        constexpr Component component = (Component)i;
        const auto& data_r = *reinterpret_cast<const typename Comp<component>::Data*>(data);
        const auto comp_name = std::string(*component);

        if (name == comp_name)
        {
            // Found component
            const auto map = Comp<component>::getTable(data_r);
            table.fromTable(map);
            table.set("name", comp_name);
            found = true;
        }
    });

    if (!found)
    {
        table.set("good", false);
        table.set("what", std::string("Component Type Doesn't Exist"));
    }
    else
        table.set("good", true);
    
    return table;
}

struct Script
{
    std::unique_ptr<Lua::Runtime> runtime;
};

namespace Engine
{
    inline static flecs::world world;

    struct Time : LuaLib::Library, Singleton<Time>
    {
        static int deltaTime(lua_State* L)
        {
            assert(!lua_gettop(L));
            const auto dt = Time::get().getDeltaTime();
            lua_pushnumber(L, (float)dt);
            return 1;
        }

        void tick()
        {
            const auto now = std::chrono::high_resolution_clock::now(); 
            _deltaTime = std::chrono::duration_cast<std::chrono::microseconds>
                (now - start).count() / 1e6;
            start = now;
        }

        double getDeltaTime() const { return _deltaTime; }

    private:
        friend class Singleton<Time>;

        std::chrono::steady_clock::time_point start;
        double _deltaTime;

        Time() :
            start(std::chrono::high_resolution_clock::now()),
            _deltaTime(0.0),
            Library("Time", 
            {
                { "deltaTime", Time::deltaTime }
            })
        {   }
    };

    struct Entity : LuaLib::Library
    {
        static int getComponent(lua_State* L)
        {
            const auto [entity_table, component] = extractArgs<Lua::Table, std::string>(L);
            const auto e_id = *entity_table.get<uint64_t*>("id");

            flecs::entity entity(world.c_ptr(), e_id);

            bool found = false;
            uint64_t c_id;
            for (uint32_t i = 0; i < entity.type().count(); i++)
            {
                std::string name = entity.type().get(i).str().c_str();
                if (name == "Comp<Component::" + component + ">.Data")
                {
                    found = true;
                    c_id = entity.type().get(i).raw_id();
                    break;
                }
            }

            if (!found)
            {
                Lua::Table ret;
                ret.set("good", false);
                ret.set("what", std::string("Entity doesn't have component"));
                ret.toStack(L);
                return 1;
            }
        
            const void* comp = entity.get(c_id);
            auto ret = componentToTable(component, comp);
            ret.set("good", true);

            ret.toStack(L);

            return 1;
        }

        static int setComponent(lua_State* L)
        {
            const auto [entity_table, component_table] = extractArgs<Lua::Table, Lua::Table>(L);

            assert(component_table.get<Lua::Boolean>("good"));

            // Get the entity
            flecs::entity entity(world.c_ptr(), *entity_table.get<uint64_t*>("id"));

            // Make sure the component is in the entity
            const auto component = component_table.get<Lua::String>("name");

            bool found = false;
            uint64_t c_id;
            for (uint32_t i = 0; i < entity.type().count(); i++)
            {
                std::string name = entity.type().get(i).str().c_str();
                if (name == "Comp<Component::" + component + ">.Data")
                {
                    found = true;
                    c_id = entity.type().get(i).raw_id();
                    break;
                }
            }

            if (!found) lua_pushboolean(L, false);
            else
            {
                void* data = entity.get_mut(c_id);
                if (!tableToComponent(component_table, data))
                    lua_pushboolean(L, false);
                else
                    lua_pushboolean(L, true);
            }
            
            return 1;
        }

        Entity() : Library("Entity",
            {
                { "getComponent", Entity::getComponent },
                { "setComponent", Entity::setComponent }
            })
        {   }
    };

    static Lua::Runtime createRuntime(const std::string& filename)
    {
        Lua::Runtime runtime(filename);
        assert(runtime);

        using namespace LuaLib;

        Debug().registerFunctions(runtime);
        Time::get().registerFunctions(runtime);

        return runtime;
    }
}

int main()
{
    auto c_log   = spdlog::stdout_color_mt("engine");
    auto lua_log = spdlog::stdout_color_mt("lua");

    auto entity = Engine::world.entity()
        .set(Comp<Component::Position>::Data{ .x = 100.f, .y = 100.f })
        .set(Comp<Component::RigidBody>::Data{ .velocity = { 50.f, -25.f } })
        .set(Script{ .runtime = std::make_unique<Lua::Runtime>(Engine::createRuntime(SOURCE_DIR "/scripts/entity.lua")) });

    c_log->info("Created entity with id {}", (uint64_t)entity.raw_id());


    // Execute the Start methods of the scripts
    auto scripts = Engine::world.filter<Script>();
    scripts.each([](Script& script)
    {
        script.runtime->runFunction<>("Start");
    });

    // Create Script update system
    Engine::world.system<Script>()
        .each([&](flecs::entity e, Script& s)
        {
            Lua::Table ent;
            ent.set("id", e.raw_id());
            ent.set("getComponent", (lua_CFunction)Engine::Entity::getComponent);
            ent.set("setComponent", (lua_CFunction)Engine::Entity::setComponent);
            s.runtime->runFunction<>("Update", ent);
        });

    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "Hello");

    Engine::world.system<const Comp<Component::Position>::Data>()
        .each([&](const Comp<Component::Position>::Data& position)
        {
            sf::RectangleShape rect;
            rect.setSize({ 10.f, 10.f });
            rect.setOrigin(rect.getSize() / 2.f);
            rect.setFillColor(sf::Color::White);
            rect.setPosition(sf::Vector2f(position.x, position.y));
            window.draw(rect);
        });

    sf::Clock timer;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        Engine::Time::get().tick();
        Engine::world.progress();
        window.display();

        window.setTitle((std::stringstream() << std::fixed << std::setprecision(2) << 1.f / timer.restart().asSeconds()).str());
    }
}

int main1()
{
    auto runtime = Engine::createRuntime(SOURCE_DIR "/scripts/test.lua");

    auto c_log   = spdlog::stdout_color_mt("engine");
    auto lua_log = spdlog::stdout_color_mt("lua");

    c_log->trace("Starting Engine");
    runtime.runFunction<>("Start");
    
    const auto [name, level] = [&]()
    {
        const auto table_res = runtime.get<Lua::Table>("Player");
        if (!table_res) { c_log->critical("Table failed: {}", (int)table_res.error().code()); exit(1); }
        const auto& table = table_res.value();
        return std::pair(table.get<Lua::String>("Name"), table.get<Lua::Number>("Level"));
    }();

    c_log->info("Player: {} [Lvl: {}]", name, level);
    
    c_log->trace("Destroying Engine");
    return 0;
}
