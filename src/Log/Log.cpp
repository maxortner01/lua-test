#include <Simple2D/Log/Log.hpp>

namespace S2D::Log
{
    Logger& Logger::instance(const std::string& name)
    {
        if (loggers.count(name)) return *loggers.at(name);

        auto logger = std::make_shared<Logger>(name);
        loggers.insert(std::pair(name, logger));
        return *logger;
    }

    spdlog::logger* Logger::operator->() const
    {
        return spdlog::get(_name).get();
    }

    Logger::Logger(const std::string& name) :
        _name(name)
    {
        auto logger = spdlog::stdout_color_mt(name);
        logger->set_level(spdlog::level::trace);
    }

    void Logger::destroy(const std::string& name)
    {
        if (!loggers.count(name)) return;
        loggers.erase(name);
    }

    void Logger::destroy()
    {
        loggers.clear();
    }
}