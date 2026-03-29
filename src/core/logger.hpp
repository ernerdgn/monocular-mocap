#pragma once
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

namespace mocap
{

struct LogEntry
{
    spdlog::level::level_enum level;
    std::string text;
};

class Logger
{
  public:
    static void init();

    static std::vector<LogEntry> getLogs();
    static void clearLogs();
};

} // namespace mocap

// macro defs
#define MOCAP_TRACE(...)    spdlog::trace(__VA_ARGS__)
#define MOCAP_INFO(...)     spdlog::info(__VA_ARGS__)
#define MOCAP_WARN(...)     spdlog::warn(__VA_ARGS__)
#define MOCAP_ERROR(...)    spdlog::error(__VA_ARGS__)
#define MOCAP_CRITICAL(...) spdlog::critical(__VA_ARGS__)