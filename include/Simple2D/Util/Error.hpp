#pragma once

#include "../Log.hpp"

namespace S2D::Util
{
    struct ErrorManager
    {
        template<typename... Args>
        static void Assert(bool expr, const std::string& message, Args&&... args)
        {
            if (!expr)
            {
                Log::Logger::instance("engine")->critical("Assert triggered: " + message, std::forward<Args>(args)...);
                std::terminate();
            }
        }
    };
}