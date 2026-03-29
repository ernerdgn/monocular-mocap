#pragma once
#include "core/result.hpp"
#include "core/logger.hpp"
#include "core/concurrent_queue.hpp"
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <optional>

namespace mocap {

    class Camera {
        public:
        Camera();
        ~Camera();

        Result<void> open(int device_id = 0);
        void close();

        std::optional<cv::Mat> getLatestFrame();

        private:
        void captureLoop(); // background worker function

        cv::VideoCapture m_capture;
        bool m_isOpen;

        std::jthread m_workerThread;
        std::atomic<bool> m_isRunning;
        ConcurrentQueue<cv::Mat> m_frameQueue;
    };

} // namespace mocap