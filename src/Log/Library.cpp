#include <Simple2D/Log/Library.hpp>
#include <Simple2D/Log/Log.hpp>
#include <Simple2D/Def.hpp>

#include <fstream>
#include <cassert>
#include <filesystem>

#include "../Lua/Lua.cpp"

namespace S2D::Log
{

int Library::log(Lua::State L)
{
    using Map = Lua::CompileTime::TypeMap<Lua::String>;

    S2D_ASSERT(lua_gettop(STATE) == 1, "Lua argument size mismatch");
    S2D_ASSERT(Map::check(L), "Not a string");

    auto val = Map::construct(STATE);

    Logger::instance("lua")->info(val);

    return 0;
}

int Library::error(Lua::State L)
{
    using Map = Lua::CompileTime::TypeMap<Lua::String>;

    S2D_ASSERT(lua_gettop(STATE) == 1, "Lua argument size mismatch");
    S2D_ASSERT(Map::check(L), "Not a string");

    auto val = Map::construct(L);

    Logger::instance("lua")->error(val);

    return 0;
}

int Library::_assert(Lua::State L)
{
    S2D_ASSERT(lua_gettop(STATE) == 1, "Lua argument size mismatch");
    S2D_ASSERT(lua_isboolean(STATE, -1), "Not a boolean");

    lua_Debug debug;
    lua_getstack(STATE, 1, &debug);
    lua_getinfo(STATE, "nSl", &debug);

    bool expr = lua_toboolean(STATE, -1);
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
            S2D_ASSERT(file, "Error opening Lua file");

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
