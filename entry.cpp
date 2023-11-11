#include <Simple2D/Simple2D.hpp>

using namespace S2D;

#ifndef SOURCE_DIR
#define SOURCE_DIR ""
#endif

int main()
{
    Lua::Runtime runtime(SOURCE_DIR "/scripts/new.lua");
    runtime.runFunction<>("Test");
    return 0;
}