#include <Simple2D/Log/Library.hpp>
#include <Simple2D/Log/Log.hpp>

#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>

namespace S2D::Log
{

int Library::log(Lua::State L)
{
    using Map = Lua::CompileTime::TypeMap<Lua::String>;

    assert(lua_gettop(L) == 1);
    assert(Map::check(L));

    auto val = Map::construct(L);

    Logger::instance("lua")->info(val);

    return 0;
}

int Library::error(Lua::State L)
{
    using Map = Lua::CompileTime::TypeMap<Lua::String>;

    assert(lua_gettop(L) == 1);
    assert(Map::check(L));

    auto val = Map::construct(L);

    Logger::instance("lua")->error(val);

    return 0;
}

int Library::_assert(Lua::State L)
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

        Logger::instance("lua")->critical("Assertion ({}) failed in file {} on line {}.", line, path.filename().c_str(), debug.currentline);
        exit(1);
    }

    return 0;
}

Library::Library() :
    Base("Debug",
    {
        { "log",      Library::log      },
        { "error",    Library::error    },
        { "assert",   Library::_assert  }
    })
{   }
    
}