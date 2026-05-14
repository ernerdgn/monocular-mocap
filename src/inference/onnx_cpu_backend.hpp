#pragma once
#include "i_inference_backend.hpp"
#include <onnxruntime_cxx_api.h>
#include <string>
#include <vector>

namespace mocap {

class ONNXCPUBackend : public IInferenceBackend 
{
public:
    ONNXCPUBackend();
    ~ONNXCPUBackend() override = default;

    Result<void> initialize(const std::string& model_path) override;
    Result<std::vector<std::vector<float>>> forward(const std::vector<std::vector<float>>& inputs) override;

private:
    Ort::Env m_env;
    Ort::SessionOptions m_sessionOptions;
    std::unique_ptr<Ort::Session> m_session;
    Ort::MemoryInfo m_memoryInfo;

    std::vector<std::string> m_inputNames;
    std::vector<std::string> m_outputNames;
    std::vector<const char*> m_inputNamesCStr;
    std::vector<const char*> m_outputNamesCStr;
    
    std::vector<std::vector<int64_t>> m_inputShapes;
    std::vector<std::vector<int64_t>> m_outputShapes;
};

}