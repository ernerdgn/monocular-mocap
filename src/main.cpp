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
    if (!std::filesystem::exists(config_path))
        config_path = "../../config.json";
    auto config_res = mocap::Config::load(config_path);
    auto cfg        = config_res.is_ok() ? config_res.value() : mocap::Config();

    mocap::WindowManager window;
    if (!window.initialize(1600, 900, "Monocular MoCap Tool").is_ok())
        return -1;

    mocap::CaptureThread captureSystem;
    mocap::Texture cameraTexture;
    
    // get model path
    std::string modelPath = "yolov8n-pose.onnx";
    if (!std::filesystem::exists(modelPath))
    {
        modelPath = "../../yolov8n-pose.onnx"; // check root from build/Debug
    }
    
    if (!std::filesystem::exists(modelPath))
    {
        modelPath = "../yolov8n-pose.onnx"; // check root from build/
    }

    std::unique_ptr<mocap::DetectionThread> detectionSystem;
    
    try
    {
        // use resolved path
        auto detector = std::make_unique<mocap::OnnxDetector>(modelPath);
        detectionSystem = std::make_unique<mocap::DetectionThread>(captureSystem, std::move(detector));
        detectionSystem->start();
    }
    catch (const std::exception& e)
    {
        MOCAP_ERROR("AI Initialization Failed: {}", e.what());
        MOCAP_WARN("Application will continue without AI capabilities.");
    }

    // main_ui
    mocap::MainUI appUI(captureSystem, cameraTexture, cfg.camera.device_id);

    MOCAP_INFO("Entering main application loop...");

    while (!window.shouldClose())
    {
        window.beginFrame();

        appUI.render();

        window.endFrame();
    }

    captureSystem.stop();
    NFD::Quit();
    MOCAP_INFO("System shutdown complete.");
    if (detectionSystem) detectionSystem->stop();
    return 0;
}