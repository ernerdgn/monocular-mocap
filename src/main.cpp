#include "core/logger.hpp"
#include "core/config.hpp"
#include "capture/camera.hpp"
#include "ui/window_manager.hpp"
#include "ui/texture.hpp"
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
    mocap::Texture cameraTexture;

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
        // check for a new frame
        auto maybe_frame = cam.getLatestFrame();
        if (maybe_frame.has_value())
        {
            cameraTexture.update(maybe_frame.value());
        }

        if (cameraTexture.isValid())
        {
            ImVec2 avail_size = ImGui::GetContentRegionAvail();
            
            float aspect = (float)cameraTexture.getWidth() / (float)cameraTexture.getHeight();
            ImVec2 render_size;
            if (avail_size.x / aspect <= avail_size.y)
            {
                render_size.x = avail_size.x;
                render_size.y = avail_size.x / aspect;
            }
            else
            {
                render_size.y = avail_size.y;
                render_size.x = avail_size.y * aspect;
            }

            ImVec2 cursor_pos = ImGui::GetCursorPos();
            cursor_pos.x += (avail_size.x - render_size.x) * 0.5f;
            cursor_pos.y += (avail_size.y - render_size.y) * 0.5f;
            ImGui::SetCursorPos(cursor_pos);

            ImTextureID tex_id = (ImTextureID)(intptr_t)cameraTexture.getId();
            ImGui::Image(tex_id, render_size);
        }
        else
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Waiting for camera...");
        }
        
        ImGui::End();

        window.endFrame();
    }

    cam.close();
    MOCAP_INFO("System shutdown complete.");
    return 0;
}