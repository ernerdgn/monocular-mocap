#pragma once
#include "capture_source.hpp"
#include <string>

namespace mocap {

    class FileSource : public ICaptureSource
    {
        public:
        explicit FileSource(const std::string& filePath);
        ~FileSource() override;

        Result<void> open() override;
        void close() override;
        bool isOpen() const override;
        Result<cv::Mat> readFrame() override;

        double getFPS() const override;
        int getWidth() const override;
        int getHeight() const override;

        private:
        std::string m_filePath;
        cv::VideoCapture m_capture;
    };

}