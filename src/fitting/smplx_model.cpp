#include "smplx_model.hpp"
#include "core/logger.hpp"

namespace mocap {

SMPLXModel::SMPLXModel(std::unique_ptr<IInferenceBackend> backend)
    : m_backend(std::move(backend)) 
{
}

Result<void> SMPLXModel::initialize(const std::string& model_path) 
{
    if (!m_backend)
    {
        return Result<void>("Inference backend is null.");
    }

    auto initRes = m_backend->initialize(model_path);
    if (!initRes.is_ok())
    {
        return Result<void>("Failed to load SMPL-X model: " + initRes.error());
    }

    m_isInitialized = true;
    MOCAP_INFO("SMPL-X Model initialized successfully.");
    return Result<void>();
}

std::vector<float> SMPLXModel::flattenRotations(const std::vector<glm::quat>& rotations) 
{
    std::vector<float> flat_data;
    flat_data.reserve(rotations.size() * 4);
    
    for (const auto& q : rotations)
    {
        flat_data.push_back(q.x);
        flat_data.push_back(q.y);
        flat_data.push_back(q.z);
        flat_data.push_back(q.w);
    }
    return flat_data;
}

Result<std::vector<JointPose>> SMPLXModel::computeForwardKinematics(
    const glm::vec3& root_translation,
    const std::vector<glm::quat>& body_rotations) 
{
    if (!m_isInitialized)
    {
        return Result<std::vector<JointPose>>("SMPLXModel not initialized.");
    }

    // prepare the tnput tensor (69 floats: 3 Transl, 66 axis-angle)
    std::vector<float> input_tensor;

    for (int i = 0; i < 10; ++i)  // betas
    {
        input_tensor.push_back(m_shapeParameters[i]);
    }

    input_tensor.push_back(root_translation.x);
    input_tensor.push_back(root_translation.y);
    input_tensor.push_back(root_translation.z);

    // convert quaternions to axis-Angle format for the nn
    for (size_t i = 0; i < 22; ++i)
    {
        glm::vec3 axis_angle(0.0f);
        if (i < body_rotations.size()) {
            float angle = glm::angle(body_rotations[i]);
            
            // math safety
            if (std::abs(angle) > 0.0001f)
            {
                glm::vec3 axis = glm::axis(body_rotations[i]);
                axis_angle = axis * angle;
            }
        }
        input_tensor.push_back(axis_angle.x);
        input_tensor.push_back(axis_angle.y);
        input_tensor.push_back(axis_angle.z);
    }

    std::vector<std::vector<float>> inputs = { input_tensor };

    // exec nn
    auto forward_res = m_backend->forward(inputs);
    if (!forward_res.is_ok()) {
        MOCAP_CRITICAL("SMPL-X Inference Failed: {}", forward_res.error()); // <-- NEW ALARM
        return Result<std::vector<JointPose>>("FK Inference Failed: " + forward_res.error());
    }

    // parse output tensor
    const auto& outputs = forward_res.value();
    if (outputs.empty() || outputs[0].empty()) {
        MOCAP_CRITICAL("SMPL-X Error: Network returned empty output.");     // <-- NEW ALARM
        return Result<std::vector<JointPose>>("Network returned empty output.");
    }

    const std::vector<float>& flat_positions = outputs[0];
    std::vector<JointPose> final_skeleton;
    
    // safety check: smplx should output 144 joints (432 floats)
    if (flat_positions.size() < 127 * 3)
    {
        MOCAP_CRITICAL("SMPL-X Error: Shape mismatch! Expected at least 381 floats, got {}", flat_positions.size());
        return Result<std::vector<JointPose>>("Output tensor is missing joints.");
    }

    // map smplx anatomical joints down to our 17 coco joints
    std::vector<int> smplx_to_coco = {
        15, 15, 15, 15, 15, // 0-4: Face/Head approximated by the neck base
        16, 17,             // 5-6: Shoulders
        18, 19,             // 7-8: Elbows
        20, 21,             // 9-10: Wrists
        1, 2,               // 11-12: Hips
        4, 5,               // 13-14: Knees
        7, 8                // 15-16: Ankles
    };

    for (int smplx_idx : smplx_to_coco)
    {
        JointPose jp;
        jp.position = glm::vec3(
            flat_positions[smplx_idx * 3 + 0],
            flat_positions[smplx_idx * 3 + 1],
            flat_positions[smplx_idx * 3 + 2]
        );
        jp.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        final_skeleton.push_back(jp);
    }

    // head fix
    // push the nose (0) and ears (3, 4) outward from the neck (15) 
    // to form a visible 3D head structure for the OpenGL lines
    if (final_skeleton.size() >= 5) {
        final_skeleton[0].position.y += 0.20f; // Nose goes up
        
        final_skeleton[3].position.y += 0.10f; // Left ear goes up
        final_skeleton[3].position.x -= 0.08f; // Left ear goes left
        
        final_skeleton[4].position.y += 0.10f; // Right ear goes up
        final_skeleton[4].position.x += 0.08f; // Right ear goes right
    }

    return Result<std::vector<JointPose>>(final_skeleton);
}

void SMPLXModel::calibrateShape(const DetectionResult& detection)
{
    if (m_isShapeCalibrated) return;

    // safety check for yolo seeing the torso
    if (detection.bodyJoints.size() < 13) return;
    
    const auto& leftShoulder = detection.bodyJoints[5];
    const auto& rightShoulder = detection.bodyJoints[6];
    const auto& leftHip = detection.bodyJoints[11];
    const auto& rightHip = detection.bodyJoints[12];

    const float CONF_THRESH = 0.6f;
    if (leftShoulder.confidence > CONF_THRESH && rightShoulder.confidence > CONF_THRESH &&
        leftHip.confidence > CONF_THRESH && rightHip.confidence > CONF_THRESH)
    {
        // measure 2d pixel dist for proportions
        float shoulder_width = glm::distance(leftShoulder.position, rightShoulder.position);
        
        glm::vec2 mid_shoulder = (leftShoulder.position + rightShoulder.position) / 2.0f;
        glm::vec2 mid_hip = (leftHip.position + rightHip.position) / 2.0f;
        float torso_length = glm::distance(mid_shoulder, mid_hip);

        m_accumulatedShoulderWidth += shoulder_width;
        m_accumulatedTorsoLength += torso_length;
        m_calibrationFramesProcessed++;

        // after 30 frames (apprx. 1 sec of capture), lock the shape!
        if (m_calibrationFramesProcessed >= 30)
        {
            float avg_width = m_accumulatedShoulderWidth / 30.0f;
            float avg_length = m_accumulatedTorsoLength / 30.0f;
            
            // calculate basic ratio, typical torso(nice..) ratio is ~1.2 to 1.5
            float ratio = avg_length / std::max(avg_width, 0.001f);

            // map the ratio to smplx beta[1] (weight/width) 
            // negative vals = thinner, positive vals = wider
            // NOTE to the curious eyes: this is a basic heuristic. a full optimizer probably use a 3d scan
            m_shapeParameters[1] = (1.35f - ratio) * 5.0f; 

            m_isShapeCalibrated = true;
            MOCAP_INFO("Body Shape Calibration Complete! Ratio: {:.2f}, Beta[1]: {:.2f}", ratio, m_shapeParameters[1]);
        }
    }
}

}