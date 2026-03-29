#pragma once
#include "core/result.hpp"
#include <opencv2/opencv.hpp>

namespace mocap {

    class ICaptureSource
    {
        public:
        virtual ~ICaptureSource() = default;

        virtual Result<void> open() = 0;
        virtual void close() = 0;
        virtual bool isOpen() const = 0;
        
        // Reads a single frame synchronously
        virtual Result<cv::Mat> readFrame() = 0;

        // Metadata
        virtual double getFPS() const = 0;
        virtual int getWidth() const = 0;
        virtual int getHeight() const = 0;
    };

}