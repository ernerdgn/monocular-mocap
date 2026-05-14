#include "capture/capture_thread.hpp"
#include "core/config.hpp"
#include "core/logger.hpp"
#include "ui/main_ui.hpp"
#include "render/texture.hpp"
#include "render/window_manager.hpp"
#include "detection/onnx_detector.hpp"
#include "detection/detection_thread.hpp"
#include "flow/optical_flow_processor.hpp"
#include "inference/onnx_cpu_backend.hpp"
#include "inference/i_inference_backend.hpp"
#include "fitting/smplx_model.hpp"
#include "fitting/pose_optimizer.hpp"
#include "fitting/fitting_thread.hpp"

#include <nfd.hpp>
#include <future>
#include <chrono>
#include <filesystem>

int main()
{
    mocap::Logger::init();
    NFD::Init();

    std::string config_path = "config.json";
    if (!std::filesystem::exists(config_path)) config_path = "../../config.json";
    auto config_res = mocap::Config::load(config_path);
    auto cfg        = config_res.is_ok() ? config_res.value() : mocap::Config();

    mocap::WindowManager window;
    if (!window.initialize(1600, 900, "Monocular MoCap Tool").is_ok()) return -1;

    mocap::CaptureThread captureSystem;
    mocap::Texture cameraTexture;
    
    std::string detectionPath = cfg.detection_path;
    std::string smplxPath = cfg.smplx_path;

    if (!std::filesystem::exists(detectionPath))
    {
        if (std::filesystem::exists("../../" + detectionPath)) detectionPath = "../../" + detectionPath;
        else if (std::filesystem::exists("../" + detectionPath)) detectionPath = "../" + detectionPath;
    }
    
    if (!std::filesystem::exists(smplxPath))
    {
        if (std::filesystem::exists("../../" + smplxPath)) smplxPath = "../../" + smplxPath;
        else if (std::filesystem::exists("../" + smplxPath)) smplxPath = "../" + smplxPath;
    }

    if (!std::filesystem::exists(detectionPath))
    {
        MOCAP_CRITICAL("FATAL: Detection model weights not found at '{}'", detectionPath);
        return EXIT_FAILURE;
    }

    if (!std::filesystem::exists(smplxPath))
    {
        MOCAP_CRITICAL("FATAL: SMPL-X model weights not found at '{}'", smplxPath);
        MOCAP_CRITICAL("See docs/exporting_models.md for instructions on getting this file.");
        return EXIT_FAILURE;
    }

    std::unique_ptr<mocap::IDetector> detector;
    try
    {
        detector = std::make_unique<mocap::OnnxDetector>(detectionPath);
    }
    catch (const std::exception& e)
    {
        MOCAP_CRITICAL("AI Initialization Failed: {}", e.what());
        return EXIT_FAILURE;
    }

    mocap::DetectionThread detectionThread(captureSystem, std::move(detector));
    detectionThread.start();

    // fitting engine startup
    MOCAP_INFO("Starting 3D Fitting Engine...");
    
    // create ai backend for smplx
    auto smplxBackend = mocap::createInferenceBackend("cpu");
    
    // init smpl-x model
    auto smplxModel = std::make_shared<mocap::SMPLXModel>(std::move(smplxBackend));
    auto smplxInitRes = smplxModel->initialize(smplxPath);
    if (!smplxInitRes.is_ok())
    {
        MOCAP_CRITICAL("SMPL-X failed to load: {}", smplxInitRes.error());
        return EXIT_FAILURE;
    }

    // create optimizer and thread
    auto poseOptimizer = std::make_shared<mocap::PoseOptimizer>(smplxModel, cfg.camera);
    mocap::FittingThread fittingThread(poseOptimizer, detectionThread);
    fittingThread.start();

    // pass thread references to ui
    mocap::MainUI appUI(captureSystem, detectionThread, fittingThread, cameraTexture, cfg.camera.device_id);

    // flow processor inst.
    mocap::OpticalFlowProcessor flowProcessor;
    double lastFlowTimestamp = -1.0; 
    
    std::future<mocap::FlowResult> flowFuture;

    MOCAP_INFO("Entering main application loop...");

    while (!window.shouldClose())
    {
        window.beginFrame();

        auto frameOpt = captureSystem.getLatestFrame();

        if (frameOpt.has_value() && frameOpt.value()->timestamp > lastFlowTimestamp)
        {
            if (flowFuture.valid() && flowFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                auto flowResult = flowFuture.get();
                appUI.getState().currentMotionMagnitude = flowResult.motionMagnitude;
            }

            if (!flowFuture.valid() || flowFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                lastFlowTimestamp = frameOpt.value()->timestamp;

                mocap::CaptureFrame frameCopy;
                frameCopy.timestamp  = frameOpt.value()->timestamp;
                frameCopy.frameIndex = frameOpt.value()->frameIndex;
                frameCopy.image      = frameOpt.value()->image.clone();

                flowFuture = std::async(std::launch::async, 
                    [&flowProcessor, frameCopy = std::move(frameCopy)]() mutable {
                        return flowProcessor.process(frameCopy);
                    });
            }
        }

        appUI.render();
        window.endFrame();
    }

    // clean up
    captureSystem.stop();
    detectionThread.stop();
    fittingThread.stop();
    
    NFD::Quit();
    MOCAP_INFO("System shutdown complete.");
    return 0;
}