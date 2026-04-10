#include "status_panel.hpp"

#include <imgui.h>

namespace mocap
{

StatusPanel::StatusPanel(CaptureThread& captureSystem)
    : m_captureSystem(captureSystem)
{
}

void StatusPanel::render(ApplicationState& state)
{
    // force this window to the top-left (x:10, y:10)
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

    // auto-resize the window so it strictly hugs the text
    ImGui::Begin("System Status", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (state.currentState == AppState::IDLE || m_captureSystem.getSource() == nullptr)
    {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "State: IDLE");
        ImGui::Text("Resolution: N/A");
        ImGui::Text("Source FPS: N/A");
    }
    else
    {
        // 1-app state
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                           state.currentState == AppState::CAPTURING ? "State: LIVE CAPTURE" : "State: FILE PLAYBACK");

        int w      = m_captureSystem.getSource()->getWidth();
        int h      = m_captureSystem.getSource()->getHeight();
        double fps = m_captureSystem.getSource()->getFPS();

        // 2-resolution tier and colors
        ImVec4 resColor;
        const char* tier = "";
        if (h >= 2160)
        {
            resColor = ImVec4(0.2f, 0.8f, 1.0f, 1.0f);
            tier     = " (4K)";
        }
        else if (h >= 1080)
        {
            resColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
            tier     = " (FHD)";
        }
        else if (h >= 720)
        {
            resColor = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
            tier     = " (HD)";
        }
        else
        {
            resColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            tier     = " (SD)";
        }

        ImGui::Text("Resolution: ");
        ImGui::SameLine();
        ImGui::TextColored(resColor, "%dx%d%s", w, h, tier);

        // 3- fps
        ImGui::Text("Source FPS: %.2f", fps);
    }

    // ui render speed
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "UI Render: %.1f FPS", ImGui::GetIO().Framerate);

    ImGui::End();
}

} // namespace mocap