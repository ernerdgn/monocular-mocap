#pragma once
#include "i_detector.hpp"
#include <onnxruntime_cxx_api.h>
#include <string>

namespace mocap {

class OnnxDetector : public IDetector
{
public:
    // path will be passed from here
    explicit OnnxDetector(const std::string& modelPath);
    ~OnnxDetector() override = default;

    Result<DetectionResult> processFrame(const std::shared_ptr<CaptureFrame>& frame) override;

private:
    Ort::Env m_env;
    std::unique_ptr<Ort::Session> m_session;

    // onnx i/o metadata
    std::vector<const char*> m_inputNodeNames;
    std::vector<const char*> m_outputNodeNames;
    std::vector<int64_t> m_inputShape;
    std::vector<int64_t> m_outputShape;

    Ort::AllocatorWithDefaultOptions m_allocator;

    std::vector<std::string> m_inputNamesStrings;
    std::vector<std::string> m_outputNamesStrings;
};

}