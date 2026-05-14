#pragma once
#include "core/types.hpp"
#include "core/result.hpp"
#include "inference/i_inference_backend.hpp"

#include <memory>
#include <vector>
#include <string>

namespace mocap {

class SMPLXModel 
{
public:
    // pass ai backend
    explicit SMPLXModel(std::unique_ptr<IInferenceBackend> backend);
    ~SMPLXModel() = default;

    // load the weights
    Result<void> initialize(const std::string& model_path);

    // return final 3d joints
    Result<std::vector<JointPose>> computeForwardKinematics(
        const glm::vec3& root_translation,
        const std::vector<glm::quat>& body_rotations
    );

    bool isShapeCalibrated() const { return m_isShapeCalibrated; }
    void calibrateShape(const DetectionResult& detection);

    // get shape params for tests
    const std::array<float, 10>& getShapeParameters() const { return m_shapeParameters; }

private:
    bool m_isShapeCalibrated = false;
    int m_calibrationFramesProcessed = 0;
    std::array<float, 10> m_shapeParameters = {0}; // 10 smplx beta coefficients

    // accumulators
    float m_accumulatedShoulderWidth = 0.0f;
    float m_accumulatedTorsoLength = 0.0f;

    std::unique_ptr<IInferenceBackend> m_backend;
    bool m_isInitialized = false;

    std::vector<float> flattenRotations(const std::vector<glm::quat>& rotations);
};

} // namespace mocap