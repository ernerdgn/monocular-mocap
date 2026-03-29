#include "core/logger.hpp"
#include "core/config.hpp"
#include "capture/camera.hpp"
#include "ui/window_manager.hpp"
#include <filesystem>
#include <imgui.h>

int main()
{
    mocap::Logger::Init();
    
    std::string config_path = "config.json";
    if (!std::filesystem::exists(config_path))
    {
        config_path = "../../config.json"; 
    }
    auto config_res = mocap::Config::Load(config_path);
    auto cfg = config_res.is_ok() ? config_res.value() : mocap::Config();

    // 2. Initialize the Window & UI
    mocap::WindowManager window;
    auto win_res = window.Initialize(1600, 900, "Monocular MoCap Tool");
    if (!win_res.is_ok()) {
        MOCAP_CRITICAL("Window Error: {}", win_res.error());
        return -1;
    }

    // 3. Open Camera
    mocap::Camera cam;
    auto open_result = cam.Open(cfg.camera.device_id);
    if (!open_result.is_ok()) {
        MOCAP_ERROR("Camera Warning: {}", open_result.error());
        // We won't crash here; we still want to show the UI even if the camera fails!
    }

    MOCAP_INFO("Entering main application loop...");

    // 4. Main Render Loop
    while (!window.ShouldClose()) {
        // Start the UI frame
        window.BeginFrame();

        // --- UI PANELS GO HERE ---
        
        // Example: Controls Panel
        ImGui::Begin("Controls");
        ImGui::Text("Application State: IDLE");
        ImGui::Separator();
        if (ImGui::Button("Start Capture")) {
            MOCAP_INFO("Capture button pressed!");
        }
        ImGui::End();

        // Example: Camera Feed Panel
        ImGui::Begin("Camera Feed");
        if (cam.ReadFrame().is_ok()) {
            ImGui::Text("Camera is actively reading frames behind the scenes.");
            ImGui::Text("Next step: Convert cv::Mat to an OpenGL Texture!");
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Camera Offline.");
        }
        ImGui::End();

        // -------------------------

        // Render the frame to the screen
        window.EndFrame();
    }

    cam.Close();
    MOCAP_INFO("System shutdown complete.");
    return 0;
}