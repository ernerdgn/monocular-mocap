#include "onnx_detector.hpp"
#include "core/logger.hpp"
#include <filesystem>

namespace mocap {

OnnxDetector::OnnxDetector(const std::string& modelPath) 
    : m_env(ORT_LOGGING_LEVEL_WARNING, "MocapONNX")
{
    
    MOCAP_INFO("Initializing ONNXRuntime environment...");
    
    try
    {
        // confix how onnx will execute the model
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(1); // using 1 thread for now to ensure stability
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        
        // convert standard string path to the format required by the os
#ifdef _WIN32
        std::wstring osModelPath = std::filesystem::path(modelPath).wstring();
        m_session = std::make_unique<Ort::Session>(m_env, osModelPath.c_str(), sessionOptions);
#else
        m_session = std::make_unique<Ort::Session>(m_env, modelPath.c_str(), sessionOptions);
#endif
        // extract model metadata
        // get input info
        size_t numInputNodes = m_session->GetInputCount();
        for (size_t i = 0; i < numInputNodes; i++)
        {
            auto inputName = m_session->GetInputNameAllocated(i, m_allocator);
            m_inputNamesStrings.push_back(inputName.get());
            m_inputNodeNames.push_back(m_inputNamesStrings.back().c_str());

            Ort::TypeInfo typeInfo = m_session->GetInputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            m_inputShape = tensorInfo.GetShape();
            
            // handle dynamic batch sizes (sometimes exported as -1)
            if (m_inputShape[0] == -1) m_inputShape[0] = 1;
        }

        // get output info
        size_t numOutputNodes = m_session->GetOutputCount();
        for (size_t i = 0; i < numOutputNodes; i++)
        {
            auto outputName = m_session->GetOutputNameAllocated(i, m_allocator);
            m_outputNamesStrings.push_back(outputName.get());
            m_outputNodeNames.push_back(m_outputNamesStrings.back().c_str());

            Ort::TypeInfo typeInfo = m_session->GetOutputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            m_outputShape = tensorInfo.GetShape();
            
            if (m_outputShape[0] == -1) m_outputShape[0] = 1;
        }

        MOCAP_INFO("Loaded ONNX Model: {}", modelPath);
        MOCAP_INFO("Expected Input Shape: [{}, {}, {}, {}]", m_inputShape[0], m_inputShape[1], m_inputShape[2], m_inputShape[3]);        
    }
    catch (const Ort::Exception& e)
    {
        // if file doesnt exist (or corrupt) throw exception
        MOCAP_ERROR("Failed to load ONNX model: {}", e.what());
        throw std::runtime_error(std::string("ONNX Runtime Error: ") + e.what());
    }
}

Result<DetectionResult> OnnxDetector::processFrame(const std::shared_ptr<CaptureFrame>& frame)
{
    if (!frame || frame->image.empty())
    {
        return Result<DetectionResult>::err("Empty frame provided to detector");
    }
    
    DetectionResult emptyResult;
    emptyResult.timestamp = frame->timestamp;
    emptyResult.frameIndex = frame->frameIndex;
    
    return Result<DetectionResult>::ok(emptyResult);
}

}