#pragma once
#include "i_detector.hpp"
#include <onnxruntime_cxx_api.h>
#include <string>
#include <vector>
#include <memory>

namespace mocap {

struct ImageInfo
{
    int originalWidth;
    int originalHeight;
    float scale;
    int padX;
    int padY;
};

class OnnxDetector : public IDetector
{
public:
    explicit OnnxDetector(const std::string& modelPath);
    ~OnnxDetector() override = default;

    Result<DetectionResult> processFrame(const std::shared_ptr<CaptureFrame>& frame) override;

private:
    // internal ai steps
    std::vector<float> preprocess(const cv::Mat& frame, ImageInfo& info);
    
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