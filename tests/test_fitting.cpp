#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "fitting/smplx_model.hpp"
#include "fitting/pose_optimizer.hpp"
#include "core/config.hpp"
#include <memory>

using namespace mocap;

// helper function to create a fake symmetrical yolo detection
DetectionResult createSyntheticTposeDetection() {
    DetectionResult det;
    det.frameIndex = 1;
    det.timestamp = 0.1f;
    det.overallConfidence = 1.0f;
    det.bodyJoints.resize(33); // MediaPipe size

    // set all confidences to 1.0
    for (auto& joint : det.bodyJoints)
    {
        joint.confidence = 1.0f;
    }

    // left shoulder (5) and right shoulder (6)
    det.bodyJoints[5].position = glm::vec2(0.4f, 0.2f);
    det.bodyJoints[6].position = glm::vec2(0.6f, 0.2f);

    // left hip (11) and right hip (12)
    det.bodyJoints[11].position = glm::vec2(0.45f, 0.6f);
    det.bodyJoints[12].position = glm::vec2(0.55f, 0.6f);

    return det;
}

TEST_CASE("SMPLXModel Body Shape Estimation", "[fitting]") {
    SMPLXModel model(nullptr); 
    
    DetectionResult synthetic_data = createSyntheticTposeDetection();

    SECTION("Model starts uncalibrated") {
        REQUIRE(model.isShapeCalibrated() == false);
    }

    SECTION("Model calibrates exactly after 30 frames") {
        // feed it 29 frames
        for (int i = 0; i < 29; ++i) {
            model.calibrateShape(synthetic_data);
        }
        REQUIRE(model.isShapeCalibrated() == false);

        // frame 30 should trigger the lock
        model.calibrateShape(synthetic_data);
        REQUIRE(model.isShapeCalibrated() == true);

        // verify math: 
        // shoulder width = 0.2, torso length = 0.4 -> ratio = 2.0
        // beta[1] = (1.35 - 2.0) * 5.0 = -3.25
        float expected_beta = (1.35f - 2.0f) * 5.0f;
        REQUIRE(model.getShapeParameters()[1] == Catch::Approx(expected_beta).margin(0.01f));
    }
}

TEST_CASE("PoseOptimizer Mode Switching and Translation", "[fitting]") {
    // setup dummy dependencies
    CameraConfig cam_cfg{1920, 1080, 60};
    auto model = std::make_shared<SMPLXModel>(nullptr);
    PoseOptimizer optimizer(model, cam_cfg);

    DetectionResult det = createSyntheticTposeDetection();
    FlowResult empty_flow;
    empty_flow.motionMagnitude = 0.0f;

    SECTION("Root Translation anchors to Center of Mass") {
        PoseFrame frame = optimizer.optimizeFrame(det, empty_flow, FittingMode::LIVE);
        
        // hips are at x: (0.45 + 0.55)/2 = 0.50
        // y: (0.60 + 0.60)/2 = 0.60
        // expected 3d trans.: x = (0.5 - 0.5)*3 = 0.0
        // y = -(0.6 - 0.5)*3 = -0.3
        
        REQUIRE(frame.root_translation.x == Catch::Approx(0.0f).margin(0.001f));
        REQUIRE(frame.root_translation.y == Catch::Approx(-0.3f).margin(0.001f));
        REQUIRE(frame.root_translation.z == Catch::Approx(3.0f).margin(0.001f)); // Default Z
    }
}