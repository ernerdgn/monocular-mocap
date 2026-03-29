#include "core/logger.hpp"
#include "core/config.hpp"
#include "capture/capture_thread.hpp"
#include "ui/window_manager.hpp"
#include "ui/texture.hpp"
#include "ui/main_ui.hpp"
#include <filesystem>
#include <nfd.hpp>

int main()
{
    mocap::Logger::init();
    NFD::Init();
    
    std::string config_path = "config.json";
    if (!std::filesystem::exists(config_path)) config_path = "../../config.json"; 
    auto config_res = mocap::Config::load(config_path);
    auto cfg = config_res.is_ok() ? config_res.value() : mocap::Config();

    mocap::WindowManager window;
    if (!window.initialize(1600, 900, "Monocular MoCap Tool").is_ok()) return -1;

    mocap::CaptureThread captureSystem;
    mocap::Texture cameraTexture;
    
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
    return 0;
}