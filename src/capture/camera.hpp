#pragma once
#include "core/result.hpp"
#include "core/logger.hpp"
#include <opencv2/opencv.hpp>

namespace mocap {

    class Camera
    {
        public:
        Camera();
        ~Camera();

        Result<void> Open(int device_id = 0);
        void Close();

        Result<cv::Mat> ReadFrame();

        private:
        cv::VideoCapture m_capture;
        bool m_isOpen;
    };
}