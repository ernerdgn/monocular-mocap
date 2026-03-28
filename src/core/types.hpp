#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <array>
#include <vector>

namespace mocap {

    // constants for smpl-x / hmr2 reqs
    constexpr size_t k_JointCount = 55;
    constexpr size_t k_FaceExpressionCount = 50;

    struct JointPose
    {
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 translation = glm::vec3(.0f);
    };

    struct PoseFrame
    {
        float timestamp = .0f;
        std::array<JointPose, k_JointCount> joints;
        std::array<float, k_FaceExpressionCount> faceExpressions;
        float confidence = .0f;
    };

    enum class AppState { IDLE, CAPTURING, REVIEWING, EXPORTING, ERROR };
    enum class ExportFormat { BVH, FBX_UNITY, FBX_UNREAL, VMC_OSC };

}