extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define STATE reinterpret_cast<lua_State*>(L)