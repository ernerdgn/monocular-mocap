#include "optical_flow_processor.hpp"
#include "core/logger.hpp"
#include <cmath>

namespace mocap
{

OpticalFlowProcessor::OpticalFlowProcessor() = default;

void OpticalFlowProcessor::reset()
{
    m_prevGray.release();
    m_prevFeatures.clear();
}

FlowResult OpticalFlowProcessor::process(const CaptureFrame& frame)
{
    FlowResult result;
    result.timestamp = frame.timestamp;
    result.motionMagnitude = .0f;

    if (frame.image.empty()) return result;

    // downsample
    cv::Mat smallImage;
    int targetWidth = 480;
    int targetHeight = static_cast<int>(targetWidth * (static_cast<float>(frame.image.rows) / frame.image.cols));
    cv::resize(frame.image, smallImage, cv::Size(targetWidth, targetHeight));

    cv::Mat currentGray;
    cv::cvtColor(smallImage, currentGray, cv::COLOR_BGR2GRAY);

    if (m_prevGray.empty())
    {
        currentGray.copyTo(m_prevGray);
        return result;
    }

    if (m_prevFeatures.size() < k_minFeaturesThreshold)
    {
        cv::goodFeaturesToTrack(m_prevGray, m_prevFeatures, k_maxFeatures, k_qualityLevel, k_minDistance);
        
        //MOCAP_INFO("Feature Detection: Found {} corners to track.", m_prevFeatures.size());

        if (m_prevFeatures.empty())
        {
            currentGray.copyTo(m_prevGray);
            return result;
        }
    }

    // track features from prevGray -> currentGray
    std::vector<cv::Point2f> currentFeatures;
    std::vector<uchar> status;
    std::vector<float> err;
    cv::calcOpticalFlowPyrLK(m_prevGray, currentGray, m_prevFeatures, currentFeatures, status, err);

    float totalMagnitude = 0.0f;
    int validPoints = 0;
    std::vector<cv::Point2f> goodFeatures;

    for (size_t i = 0; i < m_prevFeatures.size(); ++i)
    {
        if (!status[i]) continue;

        float dx = currentFeatures[i].x - m_prevFeatures[i].x;
        float dy = currentFeatures[i].y - m_prevFeatures[i].y;
        totalMagnitude += std::sqrt(dx * dx + dy * dy);
        goodFeatures.push_back(currentFeatures[i]);
        ++validPoints;
    }

    //MOCAP_INFO("Tracking: ValidPoints={}, RawTotalMag={:.4f}", validPoints, totalMagnitude);

    if (validPoints > 0)
    {
        float meanMagnitude = totalMagnitude / static_cast<float>(validPoints);
        float frameDiagonal = std::sqrt(static_cast<float>(currentGray.cols * currentGray.cols + currentGray.rows * currentGray.rows));
        result.motionMagnitude = meanMagnitude / frameDiagonal * 5.0f;
    
        //MOCAP_INFO("Calculated Final Magnitude: {:.4f}", result.motionMagnitude);
    }

    result.trackedFeatures = goodFeatures;
    result.trackingStatus = status;

    currentGray.copyTo(m_prevGray);
    m_prevFeatures = goodFeatures;

    return result;
}

}