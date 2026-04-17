#pragma once
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>

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

    struct JointPose 
    {
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Local rotation relative to parent
        glm::vec3 position = glm::vec3(0.0f);                   // World or local position for FK/Mesh
    };

    struct FaceParams 
    {
        std::vector<float> expression_blendshapes; // FLAME blendshape coefficients
        glm::quat jaw_pose = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    };

    struct PoseFrame 
    {
        double timestamp = 0.0;
        int frameIndex = 0;
        
        glm::vec3 root_translation = glm::vec3(0.0f);
        std::vector<JointPose> body_joints; // SMPL-X primary skeleton
        std::vector<JointPose> hand_joints; // MANO hands (pre-allocated for v1.4)
        FaceParams face;
        
        // Quality & Tracking Metadata
        float overall_confidence = 0.0f;
        bool is_interpolated = false;       // Flag for v0.6 Smoothing/Occlusion
    };

    // ==========================================
    // RECORDING & TIMELINE TYPES (v0.7 - v0.8)
    // ==========================================

    struct SegmentSpeedControl 
    {
        double start_timestamp = 0.0;
        double end_timestamp = 0.0;
        float speed_multiplier = 1.0f;
    };

    struct CaptureSession 
    {
        std::vector<PoseFrame> frames;
        double source_fps = 30.0;
        double total_duration = 0.0;
        std::vector<SegmentSpeedControl> speed_segments;
    };

    // ==========================================
    // EXPORT TYPES (v0.9+)
    // ==========================================

    enum class ExportFormat 
    {
        BVH,
        FBX_UNITY,
        FBX_UNREAL,
        GLTF,
        VMC_OSC
    };

    struct ExportSettings 
    {
        ExportFormat format = ExportFormat::BVH;
        std::string target_filepath;
        int target_fps = 30;
        
        bool apply_speed_control = true;
        bool use_high_quality_pass = false; // toggled in v0.11
    };

}