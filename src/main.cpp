#include "core/logger.hpp"
#include "core/config.hpp"
#include "capture/camera.hpp"
#include "ui/window_manager.hpp"
#include <filesystem>
#include <imgui.h>

int main()
{
    mocap::Logger::init();
    
    std::string config_path = "config.json";
    if (!std::filesystem::exists(config_path))
    {
        config_path = "../../config.json"; 
    }
    auto config_res = mocap::Config::load(config_path);
    auto cfg = config_res.is_ok() ? config_res.value() : mocap::Config();

    mocap::WindowManager window;
    auto win_res = window.initialize(1600, 900, "Monocular MoCap Tool");
    if (!win_res.is_ok())
    {
        MOCAP_CRITICAL("Window Error: {}", win_res.error());
        return -1;
    }

    mocap::Camera cam;
    auto open_result = cam.open(cfg.camera.device_id);
    if (!open_result.is_ok())
    {
        MOCAP_ERROR("Camera Warning: {}", open_result.error());
    }

    MOCAP_INFO("Entering main application loop...");

    while (!window.shouldClose())
    {
        window.beginFrame();
        
        ImGui::Begin("Controls");
        ImGui::Text("Application State: IDLE");
        ImGui::Separator();
        if (ImGui::Button("Start Capture"))
        {
            MOCAP_INFO("Capture button pressed!");
        }
        ImGui::End();

        ImGui::Begin("Camera Feed");
        auto maybe_frame = cam.getLatestFrame();
        if (maybe_frame.has_value())
        {
            ImGui::Text("Camera is streaming in the background at max speed!");
            ImGui::Text("Frame Width: %d, Height: %d", maybe_frame->cols, maybe_frame->rows);
        }
        else
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Waiting for next frame...");
        }
        
        ImGui::End();

        window.endFrame();
    }

    cam.close();
    MOCAP_INFO("System shutdown complete.");
    return 0;
}