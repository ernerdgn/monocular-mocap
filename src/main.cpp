#include "capture/capture_thread.hpp"
#include "core/config.hpp"
#include "core/logger.hpp"
#include "ui/main_ui.hpp"
#include "ui/texture.hpp"
#include "ui/window_manager.hpp"
#include "detection/onnx_detector.hpp"
#include "detection/detection_thread.hpp"

#include <nfd.hpp>

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
    
    // get model path from config, fallback to default if missing
    std::string modelPath = cfg.detection_path.empty() ? "yolov8n-pose.onnx" : cfg.detection_path;
    
    // check dirs
    if (!std::filesystem::exists(modelPath))
    {
        if (std::filesystem::exists("../../" + modelPath)) modelPath = "../../" + modelPath;
        else if (std::filesystem::exists("../" + modelPath)) modelPath = "../" + modelPath;
    }

    std::unique_ptr<mocap::IDetector> detector;
    
    if (!std::filesystem::exists(modelPath))
    {
        // ERROR MSGS
        MOCAP_ERROR("CRITICAL: AI Model file not found at '{}'!", modelPath);
        MOCAP_ERROR("Please download the YOLOv8-pose model from:");
        MOCAP_ERROR("https://github.com/ultralytics/assets/releases/download/v8.1.0/yolov8n-pose.onnx");
        MOCAP_ERROR("And place it in the application root directory.");
        MOCAP_WARN("Application will continue without AI capabilities.");
    }
    
    else
    {
        try // create detector
        {
            detector = std::make_unique<mocap::OnnxDetector>(modelPath);
        }

        catch (const std::exception& e) // f*ck...
        {
            MOCAP_ERROR("AI Initialization Failed: {}", e.what());
        }
    }

    // create detection thread and give it to the detector
    mocap::DetectionThread detectionThread(captureSystem, std::move(detector));
    if (detectionThread.getLatestResult() || std::filesystem::exists(modelPath)) // safety check to ensure valid setup
    {
        detectionThread.start(); 
    }

    // pass thread reference to ui
    mocap::MainUI appUI(captureSystem, detectionThread, cameraTexture, cfg.camera.device_id);

    MOCAP_INFO("Entering main application loop...");

    while (!window.shouldClose())
    {
        window.beginFrame();
        appUI.render();
        window.endFrame();
    }

    // clean up
    captureSystem.stop();
    detectionThread.stop(); // stop ai thread
    
    NFD::Quit();
    MOCAP_INFO("System shutdown complete.");
    return 0;
}