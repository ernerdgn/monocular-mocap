#pragma once
#include "core/result.hpp"

#include <opencv2/opencv.hpp>

namespace mocap
{

class ICaptureSource
{
  public:
    virtual ~ICaptureSource() = default;

    virtual Result<void> open() = 0;
    virtual void close()        = 0;
    virtual bool isOpen() const = 0;

    // reads a single frame into provided matrix
    virtual Result<void> readFrame(cv::Mat& outFrame) = 0;
    
    // playback control
    virtual double getVideoDuration() const = 0; // returns duration in milliseconds
    virtual Result<void> seek(double timestampMs) = 0;

    // metadata
    virtual double getFPS() const = 0;
    virtual int getWidth() const  = 0;
    virtual int getHeight() const = 0;
};

}