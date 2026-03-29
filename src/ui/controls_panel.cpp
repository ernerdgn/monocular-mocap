#include "controls_panel.hpp"
#include "capture/camera_source.hpp"
#include "capture/file_source.hpp"
#include <imgui.h>
#include <nfd.hpp>
#include <memory>

namespace mocap {

    ControlsPanel::ControlsPanel(CaptureThread& captureSystem, AppState& state, int defaultCameraId)
        : m_captureSystem(captureSystem), m_state(state), m_selectedCameraId(defaultCameraId) {}

    void ControlsPanel::render()
    {
        ImGui::Begin("Controls");
        
        const char* stateText = "IDLE";
        if (m_state == AppState::CAPTURING) stateText = "CAPTURING (Live Camera)";
        else if (m_state == AppState::REVIEWING) stateText = "REVIEWING (Video File)";
        
        ImGui::Text("Application State: %s", stateText);
        ImGui::Separator();

        ImGui::Text("Live Camera Input");
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Device ID", &m_selectedCameraId);
        if (m_selectedCameraId < 0) m_selectedCameraId = 0; 
        
        if (ImGui::Button("Start Camera"))
        {
            auto source = std::make_unique<CameraSource>(m_selectedCameraId);
            if (m_captureSystem.start(std::move(source)).is_ok())
            {
                m_state = AppState::CAPTURING;
            }
        }

        ImGui::Separator();

        ImGui::Text("Video File Input");
        if (ImGui::Button("Open Video File"))
        {
            nfdchar_t* outPath = nullptr;
            nfdfilteritem_t filterItem[1] = { { "Video Files", "mp4,avi,mkv,mov" } };
            
            if (NFD::OpenDialog(outPath, filterItem, 1, nullptr) == NFD_OKAY)
            {
                auto source = std::make_unique<FileSource>(outPath);
                if (m_captureSystem.start(std::move(source)).is_ok())
                {
                    m_state = AppState::REVIEWING;
                }
                NFD::FreePath(outPath);
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Stop Capture / Close File"))
        {
            m_captureSystem.stop();
            m_state = AppState::IDLE;
        }

        ImGui::End();
    }
}