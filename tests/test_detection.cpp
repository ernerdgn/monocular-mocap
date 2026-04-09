#include <catch2/catch_test_macros.hpp>
#include "core/types.hpp"
#include "detection/yolov8_joint_map.hpp"
#include "detection/detection_thread.hpp"
#include "detection/i_detector.hpp"
#include "capture/capture_thread.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace mocap;

TEST_CASE("Canonical Joint Mapping covers YOLOv8 format", "[detection]")
{
    // ensure size
    REQUIRE(k_cocoToAppJoint.size() == 17);

    // test exact mappings to ensure no off-by-one errors
    REQUIRE(k_cocoToAppJoint[0] == static_cast<int>(BodyJoint::NOSE));
    REQUIRE(k_cocoToAppJoint[5] == static_cast<int>(BodyJoint::LEFT_SHOULDER));
    REQUIRE(k_cocoToAppJoint[16] == static_cast<int>(BodyJoint::RIGHT_ANKLE));
}

TEST_CASE("DetectionResult initializes with zero confidence", "[detection]")
{
    DetectionResult result;
    
    // ensure all 17 joints are pre-allocated
    REQUIRE(result.bodyJoints.size() == static_cast<size_t>(BodyJoint::COUNT));
    
    // ensure no ghost detections exist
    bool allZero = true;
    for (const auto& joint : result.bodyJoints)
    {
        if (joint.confidence > 0.0f || joint.position.x != 0.0f)
        {
            allZero = false;
        }
    }
    REQUIRE(allZero == true);
    REQUIRE(result.overallConfidence == 0.0f);
}

// dummy detector for testing thread lifecycle without needing the actual onnx file
class DummyDetector : public IDetector
{
public:
    Result<DetectionResult> processFrame(const std::shared_ptr<CaptureFrame>&) override {
        return Result<DetectionResult>::ok(DetectionResult{});
    }
};

TEST_CASE("DetectionThread shuts down cleanly", "[detection]")
{
    CaptureThread captureSystem;
    auto dummyDetector = std::make_unique<DummyDetector>();
    
    DetectionThread thread(captureSystem, std::move(dummyDetector));
    
    thread.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    REQUIRE_NOTHROW(thread.stop());
}