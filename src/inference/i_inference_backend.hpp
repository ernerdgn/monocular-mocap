#pragma once
#include "core/result.hpp"
#include <vector>
#include <string>
#include <memory>

namespace mocap {

class IInferenceBackend 
{
public:
    virtual ~IInferenceBackend() = default;

    // load model into memory and config execution provider
    virtual Result<void> initialize(const std::string& model_path) = 0;

    // take a vector of input tensors (flattened to 1d floats) and return output tensors
    virtual Result<std::vector<std::vector<float>>> forward(const std::vector<std::vector<float>>& inputs) = 0;
};

std::unique_ptr<IInferenceBackend> createInferenceBackend(const std::string& backend_type);

}