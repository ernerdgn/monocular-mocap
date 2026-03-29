#include "logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace mocap {

    void Logger::init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("MOCAP", console_sink);
        logger->set_level(spdlog::level::trace);
        spdlog::set_default_logger(logger);
    }

}