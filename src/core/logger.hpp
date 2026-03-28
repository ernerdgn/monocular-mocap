#pragma once
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace mocap {

    class Logger
    {
    public:
        static void Init()
        {
            // format - [Color][Time] LoggerName: Message
            spdlog::set_pattern("%^[%T] %n: %v%$");
            
            // color console sink
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            
            auto logger = std::make_shared<spdlog::logger>("MOCAP", console_sink);
            logger->set_level(spdlog::level::trace);
            
            spdlog::set_default_logger(logger);
        }
    };

} // namespace mocap

// macro defs
#define MOCAP_TRACE(...)    spdlog::trace(__VA_ARGS__)
#define MOCAP_INFO(...)     spdlog::info(__VA_ARGS__)
#define MOCAP_WARN(...)     spdlog::warn(__VA_ARGS__)
#define MOCAP_ERROR(...)    spdlog::error(__VA_ARGS__)
#define MOCAP_CRITICAL(...) spdlog::critical(__VA_ARGS__)