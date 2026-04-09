#include "onnx_detector.hpp"
#include "core/logger.hpp"
#include "yolov8_joint_map.hpp"
#include <filesystem>
#include <opencv2/imgproc.hpp>

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
        return Result<DetectionResult>::err("Empty frame received");
    }

    try
    {
        ImageInfo info;
        std::vector<float> inputBlob = preprocess(frame->image, info);

        // prep onnx input tensor
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, 
            inputBlob.data(), 
            inputBlob.size(), 
            m_inputShape.data(), 
            m_inputShape.size()
        );

        // run inference
        auto outputTensors = m_session->Run(
            Ort::RunOptions{nullptr},
            m_inputNodeNames.data(),
            &inputTensor,
            1,
            m_outputNodeNames.data(),
            m_outputNodeNames.size()
        );

        // post processing
        float* outputData = outputTensors[0].GetTensorMutableData<float>();
        
        //const int numFeatures = 56;  // 4 (box) + 1 (obj conf) + 17*3 (kpts)
        const int numAnchors = 8400; // yolov8n default grid size

        int bestAnchor = -1;
        float bestConf = .0f;

        // single anchor nms
        // obj confidence is feature index 4- memory layout: data[feature * anchors + anchor]
        for (int a = 0; a < numAnchors; ++a)
        {
            float conf = outputData[4 * numAnchors + a];
            if (conf > bestConf)
            {
                bestConf = conf;
                bestAnchor = a;
            }
        }

        DetectionResult result;
        
        // safety check for all joints start cleanly lost (0 confidence)
        for (auto& joint : result.bodyJoints)
        {
            joint.position = {.0f, .0f};
            joint.confidence = .0f;
        }

        // extract and transform keypoints
        const float CONFIDENCE_THRESHOLD = .3f; // TODO?can be pulled to config later
        
        if (bestAnchor >= 0 && bestConf >= CONFIDENCE_THRESHOLD)
        {
            float totalConfidence = .0f;
            int validJoints = 0;
            // layout per keypoint: X, Y, Confidence
            for (int k = 0; k < 17; ++k)
            {
                int baseFeatureIdx = 5 + (k * 3);
                
                float kx    = outputData[(baseFeatureIdx + 0) * numAnchors + bestAnchor];
                float ky    = outputData[(baseFeatureIdx + 1) * numAnchors + bestAnchor];
                float kconf = outputData[(baseFeatureIdx + 2) * numAnchors + bestAnchor];

                // invert letterbox transform
                float origX = (kx - info.padX) / info.scale;
                float origY = (ky - info.padY) / info.scale;

                // normalize to 0.0->1.0 range based on original frame size
                float normX = origX / static_cast<float>(info.originalWidth);
                float normY = origY / static_cast<float>(info.originalHeight);

                // map to app skeleton
                int appIdx = k_cocoToAppJoint[k];
                if (appIdx >= 0 && appIdx < result.bodyJoints.size()) // -1 : joint is ignored
                {
                    result.bodyJoints[appIdx].position = glm::vec2(normX, normY);
                    result.bodyJoints[appIdx].confidence = kconf;
                    
                    totalConfidence += kconf;
                    validJoints++;
                }
            }

            if (validJoints > 0) result.overallConfidence = totalConfidence / validJoints;
        }

        return Result<DetectionResult>(result);

    }
    
    catch (const std::exception& e)
    {
        MOCAP_ERROR("Detection error: {}", e.what());
        return Result<DetectionResult>::err(e.what());
    }
}

std::vector<float> OnnxDetector::preprocess(const cv::Mat& frame, ImageInfo& info)
{
    // record metadata for coord mapping
    info.originalWidth = frame.cols;
    info.originalHeight = frame.rows;

    // calculate scaling to fit 640x640 while keeping aspect ratio
    int targetSize = 640;
    info.scale = std::min(targetSize / (float)frame.cols, targetSize / (float)frame.rows);
    
    int newUnpadW = static_cast<int>(std::round(frame.cols * info.scale));
    int newUnpadH = static_cast<int>(std::round(frame.rows * info.scale));

    // calculate padding
    int padW = targetSize - newUnpadW;
    int padH = targetSize - newUnpadH;
    info.padX = padW / 2;
    info.padY = padH / 2;

    // resize
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(newUnpadW, newUnpadH), 0, 0, cv::INTER_LINEAR);

    // add padding
    cv::Mat canvas;
    int top = info.padY;
    int bottom = targetSize - newUnpadH - top;
    int left = info.padX;
    int right = targetSize - newUnpadW - left;
    cv::copyMakeBorder(resized, canvas, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));

    // bgr->rgb and normalization
    cv::Mat rgb;
    cv::cvtColor(canvas, rgb, cv::COLOR_BGR2RGB);
    rgb.convertTo(rgb, CV_32FC3, 1.0f / 255.0f);

    // hwc->chw
    // vector of 1 228 800 floats (3*640*640)
    std::vector<float> inputBlob(3 * targetSize * targetSize);
    
    // use cv to split the 3-channel image directly to vectors memory
    std::vector<cv::Mat> channels(3);
    for (int i = 0; i < 3; ++i)
    {
        channels[i] = cv::Mat(targetSize, targetSize, CV_32FC1, &inputBlob[i * targetSize * targetSize]);
    }
    cv::split(rgb, channels);

    return inputBlob;
}

}