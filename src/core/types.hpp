#pragma once
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace mocap {

    enum class AppState
    {
        IDLE,
        CAPTURING,
        REVIEWING
    };

    struct CaptureFrame
    {
        cv::Mat image;
        double timestamp = 0.0;
        int frameIndex = 0;
    };

    // standard coco 17-keypoint skeleton map
    enum class BodyJoint
    {
        NOSE = 0,
        LEFT_EYE = 1,
        RIGHT_EYE = 2,
        LEFT_EAR = 3,
        RIGHT_EAR = 4,
        LEFT_SHOULDER = 5,
        RIGHT_SHOULDER = 6,
        LEFT_ELBOW = 7,
        RIGHT_ELBOW = 8,
        LEFT_WRIST = 9,
        RIGHT_WRIST = 10,
        LEFT_HIP = 11,
        RIGHT_HIP = 12,
        LEFT_KNEE = 13,
        RIGHT_KNEE = 14,
        LEFT_ANKLE = 15,
        RIGHT_ANKLE = 16,
        COUNT = 17 // array sizing helper
    };

    // single detected 2d point on screen
    struct Joint2D
    {
        glm::vec2 position{0.0f, 0.0f}; // pixel coordinates (x, y)
        float confidence = 0.0f; // 0.0 (lost) to 1.0 (certain)
    };

    // complete output of detection phase for single frame
    struct DetectionResult
    {
        std::vector<Joint2D> bodyJoints;
        double timestamp = 0.0;
        int frameIndex = 0;
        float overallConfidence = 0.0f; // average confidence of all body joints
        
        // constructor that pre-allocs the 17 joints
        DetectionResult() : bodyJoints(static_cast<size_t>(BodyJoint::COUNT)) {}
    };

    struct FlowResult
    {
        double timestamp = 0.0;
        std::vector<cv::Point2f> trackedFeatures;
        std::vector<uchar> trackingStatus;
        float motionMagnitude = 0.0f; // .0 (still) to 1.0+ (fast moves)
    };

}