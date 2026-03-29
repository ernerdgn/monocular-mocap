#pragma once
#include "capture_source.hpp"
#include "core/concurrent_queue.hpp"
#include <memory>
#include <thread>
#include <atomic>
#include <optional>

namespace mocap {

    class CaptureThread {
        public:
        CaptureThread() : m_isRunning(false) {}
        ~CaptureThread();

        Result<void> start(std::unique_ptr<ICaptureSource> source);
        void stop();

        std::optional<cv::Mat> getLatestFrame();
        
        bool isRunning() const { return m_isRunning; }
        ICaptureSource* getSource() const { return m_source.get(); }

        private:
        void threadLoop();

        std::unique_ptr<ICaptureSource> m_source;
        std::jthread m_worker;
        std::atomic<bool> m_isRunning;
        std::unique_ptr<ConcurrentQueue<cv::Mat>> m_frameQueue;
    };

}