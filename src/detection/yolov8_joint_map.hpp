#pragma once
#include "core/types.hpp"
#include <array>

namespace mocap {

// YOLOv8-Pose COCO 17-keypoint order:
// 0: Nose, 1: L_Eye, 2: R_Eye, 3: L_Ear, 4: R_Ear, 
// 5: L_Shoulder, 6: R_Shoulder, 7: L_Elbow, 8: R_Elbow, 
// 9: L_Wrist, 10: R_Wrist, 11: L_Hip, 12: R_Hip, 
// 13: L_Knee, 14: R_Knee, 15: L_Ankle, 16: R_Ankle

static constexpr std::array<int, 17> k_cocoToAppJoint = {
    static_cast<int>(BodyJoint::NOSE),           // 0
    static_cast<int>(BodyJoint::LEFT_EYE),       // 1
    static_cast<int>(BodyJoint::RIGHT_EYE),      // 2
    static_cast<int>(BodyJoint::LEFT_EAR),       // 3
    static_cast<int>(BodyJoint::RIGHT_EAR),      // 4
    static_cast<int>(BodyJoint::LEFT_SHOULDER),  // 5
    static_cast<int>(BodyJoint::RIGHT_SHOULDER), // 6
    static_cast<int>(BodyJoint::LEFT_ELBOW),     // 7
    static_cast<int>(BodyJoint::RIGHT_ELBOW),    // 8
    static_cast<int>(BodyJoint::LEFT_WRIST),     // 9
    static_cast<int>(BodyJoint::RIGHT_WRIST),    // 10
    static_cast<int>(BodyJoint::LEFT_HIP),       // 11
    static_cast<int>(BodyJoint::RIGHT_HIP),      // 12
    static_cast<int>(BodyJoint::LEFT_KNEE),      // 13
    static_cast<int>(BodyJoint::RIGHT_KNEE),     // 14
    static_cast<int>(BodyJoint::LEFT_ANKLE),     // 15
    static_cast<int>(BodyJoint::RIGHT_ANKLE)     // 16
};

}