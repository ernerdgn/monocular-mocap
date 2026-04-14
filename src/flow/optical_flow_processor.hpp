#pragma once
#include "core/types.hpp"
#include <opencv2/core.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

namespace mocap
{
class OpticalFlowProcessor
{
  public:
    OpticalFlowProcessor();
    
    FlowResult process(const CaptureFrame& frame);

    void reset();

  private:
    cv::Mat m_prevGray;
    std::vector<cv::Point2f> m_prevFeatures;
    
    // Configuration thresholds
    const int k_maxFeatures = 100; // 200
    const double k_qualityLevel = 0.03;
    const double k_minDistance = 7.0;
    const size_t k_minFeaturesThreshold = 10; // 50
};
}