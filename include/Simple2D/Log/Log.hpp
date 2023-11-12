#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <unordered_map>
#include <memory>

#include "../Util.hpp"

namespace S2D::Log
{
    struct Logger
    {
        static Logger& instance(const std::string& name);
        static void destroy(const std::string& name);
        static void destroy();

        spdlog::logger* operator->() const;

    public:
        Logger(const std::string& name);

        std::string _name;
        inline static std::unordered_map<std::string, std::shared_ptr<Logger>> loggers;
    };
}