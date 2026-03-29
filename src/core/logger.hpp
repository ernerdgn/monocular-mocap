#pragma once
#include <memory>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>

namespace mocap {

    class Logger
    {
    public:
        static void init();

        static std::vector<std::string> getLogs();
        static void clearLogs();
    };

}

// macro defs
#define MOCAP_TRACE(...)    spdlog::trace(__VA_ARGS__)
#define MOCAP_INFO(...)     spdlog::info(__VA_ARGS__)
#define MOCAP_WARN(...)     spdlog::warn(__VA_ARGS__)
#define MOCAP_ERROR(...)    spdlog::error(__VA_ARGS__)
#define MOCAP_CRITICAL(...) spdlog::critical(__VA_ARGS__)