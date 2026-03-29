#pragma once
#include "capture_source.hpp"

namespace mocap
{

class CameraSource : public ICaptureSource
{
  public:
    explicit CameraSource(int deviceId = 0);
    ~CameraSource() override;

    Result<void> open() override;
    void close() override;
    bool isOpen() const override;
    Result<void> readFrame(cv::Mat& outFrame) override;

    double getFPS() const override;
    int getWidth() const override;
    int getHeight() const override;

    double getVideoDuration() const override;
    Result<void> seek(double timestampMs) override;

  private:
    int m_deviceId;
    cv::VideoCapture m_capture;
};

} // namespace mocap