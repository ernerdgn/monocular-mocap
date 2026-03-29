#include "logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include <mutex>

namespace mocap {

    static std::vector<LogEntry> g_logHistory;
    static std::mutex g_logMutex;

    template<typename Mutex>
    class ImGuiSink : public spdlog::sinks::base_sink<Mutex>
    {
    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            this->formatter_->format(msg, formatted);
            
            std::lock_guard<std::mutex> lock(g_logMutex);

            g_logHistory.push_back({msg.level, fmt::to_string(formatted)});
            
            if (g_logHistory.size() > 1000) {
                g_logHistory.erase(g_logHistory.begin());
            }
        }
        void flush_() override {}
    };

    void Logger::init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto imgui_sink = std::make_shared<ImGuiSink<std::mutex>>();
        
        std::vector<spdlog::sink_ptr> sinks {console_sink, imgui_sink};
        auto logger = std::make_shared<spdlog::logger>("MOCAP", sinks.begin(), sinks.end());
        
        logger->set_level(spdlog::level::trace);
        spdlog::set_default_logger(logger);
    }

    std::vector<LogEntry> Logger::getLogs()
    {
        std::lock_guard<std::mutex> lock(g_logMutex);
        return g_logHistory;
    }

    void Logger::clearLogs()
    {
        std::lock_guard<std::mutex> lock(g_logMutex);
        g_logHistory.clear();
    }
}