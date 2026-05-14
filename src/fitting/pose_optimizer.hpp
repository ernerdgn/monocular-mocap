#pragma once

#include "core/types.hpp"
#include "core/config.hpp"
#include "fitting/smplx_model.hpp"

#include <memory>
#include <vector>

namespace mocap {

class PoseOptimizer 
{
public:
    PoseOptimizer(std::shared_ptr<SMPLXModel> model, const CameraConfig& cam_config);
    ~PoseOptimizer() = default;

    PoseFrame optimizeFrame(const DetectionResult& detection, const FlowResult& flow, FittingMode mode = FittingMode::LIVE);

private:
    std::shared_ptr<SMPLXModel> m_model;
    CameraConfig m_camConfig;

    std::vector<glm::quat> m_previousRotations;
    glm::vec3 m_previousTranslation;
    
    float m_fx, m_fy, m_cx, m_cy;

    glm::vec2 projectTo2D(const glm::vec3& point3d) const;
    
    float calculateReprojectionError(
        const std::vector<JointPose>& skeleton3d, 
        const std::vector<Joint2D>& target2d) const;
};

}