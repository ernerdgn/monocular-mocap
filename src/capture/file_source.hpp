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
    
    // reads directly into the provided pooled frame
    Result<void> readFrame(cv::Mat& outFrame) override;

    double getFPS() const override;
    int getWidth() const override;
    int getHeight() const override;

    double getVideoDuration() const override;
    Result<void> seek(double timestampMs) override;

private:
    std::string m_filePath;
    cv::VideoCapture m_capture;
};

}