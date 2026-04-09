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
    
    // get model path
    std::string modelPath = "yolov8n-pose.onnx";
    if (!std::filesystem::exists(modelPath)) modelPath = "../../yolov8n-pose.onnx";
    if (!std::filesystem::exists(modelPath)) modelPath = "../yolov8n-pose.onnx";

    // create detector
    auto detector = std::make_unique<mocap::OnnxDetector>(modelPath);
    
    // create detection thread and give it to the detector
    mocap::DetectionThread detectionThread(captureSystem, std::move(detector));
    detectionThread.start(); // run!

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