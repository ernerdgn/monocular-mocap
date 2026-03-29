#pragma once
#include "core/types.hpp"
#include "core/result.hpp"
#include <memory>

namespace mocap {

class IDetector
{
public:
    virtual ~IDetector() = default;

    // takes a live camera frame and returns the 2D skeleton coordinates
    virtual Result<DetectionResult> processFrame(const std::shared_ptr<CaptureFrame>& frame) = 0;
};
}