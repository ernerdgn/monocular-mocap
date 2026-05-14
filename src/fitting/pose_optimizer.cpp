#include "pose_optimizer.hpp"
#include "core/logger.hpp"

namespace mocap {

PoseOptimizer::PoseOptimizer(std::shared_ptr<SMPLXModel> model, const CameraConfig& cam_config)
    : m_model(std::move(model)), m_camConfig(cam_config)
{
    m_fx = m_camConfig.width * 0.8f;  
    m_fy = m_fx;                      
    m_cx = m_camConfig.width / 2.0f;
    m_cy = m_camConfig.height / 2.0f;

    //m_previousRotations.resize(static_cast<size_t>(BodyJoint::COUNT), glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    m_previousRotations.resize(22, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    m_previousTranslation = glm::vec3(0.0f, 0.0f, 3.0f); // start 3 meters away from camera
}

glm::vec2 PoseOptimizer::projectTo2D(const glm::vec3& point3d) const
{
    float z = std::max(point3d.z, 0.001f); 

    // (x/z * fx + cx, y/z * fy + cy)
    float u = (point3d.x / z) * m_fx + m_cx;
    float v = (point3d.y / z) * m_fy + m_cy;

    return glm::vec2(u, v);
}

float PoseOptimizer::calculateReprojectionError(
    const std::vector<JointPose>& skeleton3d, 
    const std::vector<Joint2D>& target2d) const
{
    float total_error = 0.0f;
    float total_weight = 0.0f;

    for (size_t i = 0; i < target2d.size() && i < skeleton3d.size(); ++i) 
    {
        float confidence = target2d[i].confidence;
        if (confidence < 0.2f) continue;

        glm::vec2 projected = projectTo2D(skeleton3d[i].position);
        float distance = glm::distance(projected, target2d[i].position);
        
        total_error += (distance * confidence);
        total_weight += confidence;
    }

    return total_weight > 0.0f ? (total_error / total_weight) : 9999.0f;
}

// BONE MAP
struct IKBone {
    int smplx_joint; // rotation index
    int coco_parent; // bone start
    int coco_child;  // bone end
};

const std::vector<IKBone> BONES_TO_SOLVE = {
    {16, 5, 7},   // left shoulder -> left elbow
    {17, 6, 8},   // right shoulder -> right elbow
    {18, 7, 9},   // left elbow -> left wrist
    {19, 8, 10},  // right elbow -> right wrist
    {1, 11, 13},  // left hip -> left knee
    {2, 12, 14},  // right hip -> right knee
    {4, 13, 15},  // left knee -> left ankle
    {5, 14, 16}   // right knee -> right ankle
};


PoseFrame PoseOptimizer::optimizeFrame(const DetectionResult& detection, const FlowResult& flow, FittingMode mode)
{
    // trigger shape calibration
    if (!m_model->isShapeCalibrated()) m_model->calibrateShape(detection);

    PoseFrame frame;
    frame.timestamp = detection.timestamp;
    frame.frameIndex = detection.frameIndex;
    frame.overall_confidence = detection.overallConfidence;

    std::vector<glm::quat> current_rotations = m_previousRotations;
    glm::vec3 current_translation = m_previousTranslation;

    // root translation anchor (shared by both modes)
    const float CONF_THRESH = 0.5f;
    if (detection.bodyJoints.size() > 12) 
    {
        const auto& leftHip = detection.bodyJoints[11];
        const auto& rightHip = detection.bodyJoints[12];

        if (leftHip.confidence > CONF_THRESH && rightHip.confidence > CONF_THRESH) 
        {
            float avg_x = (leftHip.position.x + rightHip.position.x) / 2.0f;
            float avg_y = (leftHip.position.y + rightHip.position.y) / 2.0f;
            current_translation.x = (avg_x - 0.5f) * 3.0f; 
            current_translation.y = -(avg_y - 0.5f) * 3.0f; 
        }
    }

    // solver
    if (mode == FittingMode::LIVE) 
    {
        // LIVE: 1-pass analytical inverse kinematics with flow-driven smoothing
        float dynamic_alpha = 0.15f + (flow.motionMagnitude * 0.6f);
        dynamic_alpha = glm::clamp(dynamic_alpha, 0.15f, 0.85f);

        auto fk_result = m_model->computeForwardKinematics(current_translation, current_rotations);
        if (fk_result.is_ok()) 
        {
            std::vector<JointPose> current_skeleton = fk_result.value();
            for (const auto& bone : BONES_TO_SOLVE) 
            {
                const auto& yolo_parent = detection.bodyJoints[bone.coco_parent];
                const auto& yolo_child = detection.bodyJoints[bone.coco_child];

                if (yolo_parent.confidence > CONF_THRESH && yolo_child.confidence > CONF_THRESH) 
                {
                    glm::vec2 proj_parent = projectTo2D(current_skeleton[bone.coco_parent].position);
                    glm::vec2 proj_child = projectTo2D(current_skeleton[bone.coco_child].position);
                    if (glm::distance(proj_parent, proj_child) < 1.0f) continue;
                    
                    glm::vec2 v_curr = glm::normalize(proj_child - proj_parent);
                    glm::vec2 targ_parent = glm::vec2(yolo_parent.position.x * m_camConfig.width, (1.0f - yolo_parent.position.y) * m_camConfig.height);
                    glm::vec2 targ_child = glm::vec2(yolo_child.position.x * m_camConfig.width, (1.0f - yolo_child.position.y) * m_camConfig.height);
                    if (glm::distance(targ_parent, targ_child) < 1.0f) continue;
                    
                    glm::vec2 v_targ = glm::normalize(targ_child - targ_parent);
                    float dot = glm::clamp(glm::dot(v_curr, v_targ), -1.0f, 1.0f);
                    float det = v_curr.x * v_targ.y - v_curr.y * v_targ.x;
                    float angle = std::atan2(det, dot);

                    if (std::abs(angle) > 0.05f)
                    { 
                        glm::quat correction = glm::angleAxis(angle, glm::vec3(0.0f, 0.0f, 1.0f));
                        glm::quat target_rotation = correction * current_rotations[bone.smplx_joint];
                        current_rotations[bone.smplx_joint] = glm::normalize(glm::slerp(current_rotations[bone.smplx_joint], target_rotation, dynamic_alpha));
                    }
                }
            }
        }
    }
    else 
    {
        // EXPORT: multi-pass hierarchical convergence (no smoothing, no smoothie)
        const int MAX_ITERATIONS = 3; 
        for (int iter = 0; iter < MAX_ITERATIONS; ++iter) 
        {
            auto fk_result = m_model->computeForwardKinematics(current_translation, current_rotations);
            if (!fk_result.is_ok()) break;
            
            std::vector<JointPose> current_skeleton = fk_result.value();
            
            // check overall error using existing function
            float error = calculateReprojectionError(current_skeleton, detection.bodyJoints);
            if (error < 5.0f) break; // converged! ehe

            for (const auto& bone : BONES_TO_SOLVE) 
            {
                const auto& yolo_parent = detection.bodyJoints[bone.coco_parent];
                const auto& yolo_child = detection.bodyJoints[bone.coco_child];

                if (yolo_parent.confidence > CONF_THRESH && yolo_child.confidence > CONF_THRESH) 
                {
                    glm::vec2 proj_parent = projectTo2D(current_skeleton[bone.coco_parent].position);
                    glm::vec2 proj_child = projectTo2D(current_skeleton[bone.coco_child].position);
                    if (glm::distance(proj_parent, proj_child) < 1.0f) continue;
                    
                    glm::vec2 v_curr = glm::normalize(proj_child - proj_parent);
                    glm::vec2 targ_parent = glm::vec2(yolo_parent.position.x * m_camConfig.width, (1.0f - yolo_parent.position.y) * m_camConfig.height);
                    glm::vec2 targ_child = glm::vec2(yolo_child.position.x * m_camConfig.width, (1.0f - yolo_child.position.y) * m_camConfig.height);
                    if (glm::distance(targ_parent, targ_child) < 1.0f) continue;
                    
                    glm::vec2 v_targ = glm::normalize(targ_child - targ_parent);
                    float dot = glm::clamp(glm::dot(v_curr, v_targ), -1.0f, 1.0f);
                    float det = v_curr.x * v_targ.y - v_curr.y * v_targ.x;
                    float angle = std::atan2(det, dot);

                    if (std::abs(angle) > 0.01f) // tighter deadzone for export
                    {
                        glm::quat correction = glm::angleAxis(angle, glm::vec3(0.0f, 0.0f, 1.0f));
                        // exact mathematical snap, no SLERP, no drag. no drugs...
                        current_rotations[bone.smplx_joint] = glm::normalize(correction * current_rotations[bone.smplx_joint]); 
                    }
                }
            }
        }
    }

    // fin mesh
    auto final_fk = m_model->computeForwardKinematics(current_translation, current_rotations);
    if (final_fk.is_ok())
    {
        frame.body_joints = final_fk.value();
    }
    
    frame.root_translation = current_translation;
    m_previousRotations = current_rotations;
    m_previousTranslation = current_translation;

    return frame;
}

}