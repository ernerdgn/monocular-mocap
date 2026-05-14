#include "onnx_cpu_backend.hpp"
#include "core/logger.hpp"

namespace mocap {

std::unique_ptr<IInferenceBackend> createInferenceBackend(const std::string& backend_type) 
{
    // for v0.5 - only support cpu / cuda and directml will be added
    if (backend_type == "cpu" || backend_type.empty())
    {
        return std::make_unique<ONNXCPUBackend>();
    }
    MOCAP_WARN("Unknown backend type '{}', falling back to CPU", backend_type);
    return std::make_unique<ONNXCPUBackend>();
}

ONNXCPUBackend::ONNXCPUBackend()
    : m_env(ORT_LOGGING_LEVEL_WARNING, "ONNXCPUBackend"),
      m_memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
{
    m_sessionOptions.SetIntraOpNumThreads(1);
    m_sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
}

Result<void> ONNXCPUBackend::initialize(const std::string& model_path) 
{
    try 
    {
#ifdef _WIN32
        std::wstring w_model_path(model_path.begin(), model_path.end());
        m_session = std::make_unique<Ort::Session>(m_env, w_model_path.c_str(), m_sessionOptions);
#else
        m_session = std::make_unique<Ort::Session>(m_env, model_path.c_str(), m_sessionOptions);
#endif

        Ort::AllocatorWithDefaultOptions allocator;

        size_t num_inputs = m_session->GetInputCount();
        for (size_t i = 0; i < num_inputs; i++) 
        {
            auto input_name = m_session->GetInputNameAllocated(i, allocator);
            m_inputNames.push_back(input_name.get());
            
            auto type_info = m_session->GetInputTypeInfo(i);
            auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
            m_inputShapes.push_back(tensor_info.GetShape());
        }

        size_t num_outputs = m_session->GetOutputCount();
        for (size_t i = 0; i < num_outputs; i++) 
        {
            auto output_name = m_session->GetOutputNameAllocated(i, allocator);
            m_outputNames.push_back(output_name.get());
            
            auto type_info = m_session->GetOutputTypeInfo(i);
            auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
            m_outputShapes.push_back(tensor_info.GetShape());
        }

        for (const auto& name : m_inputNames) m_inputNamesCStr.push_back(name.c_str());
        for (const auto& name : m_outputNames) m_outputNamesCStr.push_back(name.c_str());

        MOCAP_INFO("ONNX CPU Backend initialized successfully: {}", model_path);
        return Result<void>();
    }
    catch (const Ort::Exception& e) 
    {
        return Result<void>(std::string("ONNX Runtime Error: ") + e.what());
    }
}

Result<std::vector<std::vector<float>>> ONNXCPUBackend::forward(const std::vector<std::vector<float>>& inputs) 
{
    if (inputs.size() != m_inputNames.size())
    {
        return Result<std::vector<std::vector<float>>>("Input count mismatch.");
    }

    try 
    {
        std::vector<Ort::Value> input_tensors;
        
        // Map C++ vectors to ONNX tensors
        for (size_t i = 0; i < inputs.size(); i++) 
        {
            // Cast away constness for ONNX API (it does not modify the memory here)
            float* input_data = const_cast<float*>(inputs[i].data());
            size_t input_size = inputs[i].size();

            input_tensors.push_back(Ort::Value::CreateTensor<float>(
                m_memoryInfo, input_data, input_size, 
                m_inputShapes[i].data(), m_inputShapes[i].size()
            ));
        }

        auto output_tensors = m_session->Run(
            Ort::RunOptions{nullptr}, 
            m_inputNamesCStr.data(), input_tensors.data(), input_tensors.size(), 
            m_outputNamesCStr.data(), m_outputNamesCStr.size()
        );

        std::vector<std::vector<float>> outputs;
        for (size_t i = 0; i < output_tensors.size(); i++) 
        {
            float* floatarr = output_tensors[i].GetTensorMutableData<float>();
            size_t count = output_tensors[i].GetTensorTypeAndShapeInfo().GetElementCount();
            outputs.emplace_back(floatarr, floatarr + count);
        }

        return Result<std::vector<std::vector<float>>>(outputs);
    }
    catch (const Ort::Exception& e) 
    {
        return Result<std::vector<std::vector<float>>>(std::string("Inference Failed: ") + e.what());
    }
}

}