#include "controls_panel.hpp"
#include "capture/camera_source.hpp"
#include "capture/file_source.hpp"
#include <imgui.h>
#include <nfd.hpp>
#include <memory>

namespace mocap {

ControlsPanel::ControlsPanel(CaptureThread& captureSystem, AppState& state, int defaultCameraId)
    : m_captureSystem(captureSystem), m_state(state), m_selectedCameraId(defaultCameraId)
    {
        m_availableCameras = DeviceEnumerator::getAvailableCameras();
    
        // ensure selected id is valid
        if (!m_availableCameras.empty() && m_selectedCameraId < 0)
        {
            m_selectedCameraId = m_availableCameras.front().id;
        }
    }

void ControlsPanel::render()
{
    ImGui::Begin("Controls");
    
    // app state
    const char* stateText = "IDLE";
    if (m_state == AppState::CAPTURING) stateText = "CAPTURING (Live Camera)";
    else if (m_state == AppState::REVIEWING) stateText = "REVIEWING (Video File)";
    
    ImGui::Text("Application State: %s", stateText);
    ImGui::Separator();

    // live cam controls
    ImGui::Text("Live Camera Input");
    ImGui::SetNextItemWidth(250);
    
    // 1- determine the name of the current device for the dropdown
    std::string preview = "Select Camera";
    for (const auto& cam : m_availableCameras)
    {
        if (cam.id == m_selectedCameraId)
        {
            preview = std::to_string(cam.id) + ": " + cam.name;
            break;
        }
    }

    // 2- render the actual dropdown
    if (ImGui::BeginCombo("Device", preview.c_str()))
    {
        for (const auto& cam : m_availableCameras)
        {
            std::string label = std::to_string(cam.id) + ": " + cam.name;
            bool is_selected = (m_selectedCameraId == cam.id);
            
            if (ImGui::Selectable(label.c_str(), is_selected))
            {
                m_selectedCameraId = cam.id;
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    // 3- start cam button
    if (ImGui::Button("Start Camera"))
    {
        auto source = std::make_unique<CameraSource>(m_selectedCameraId);
        if (m_captureSystem.start(std::move(source)).is_ok())
        {
            m_state = AppState::CAPTURING;
        }
    }

    ImGui::Separator();

    // video file controls
    ImGui::Text("Video File Input");
    if (ImGui::Button("Open Video File"))
    {
        nfdchar_t* outPath = nullptr;
        // native file dial filter for common video formats
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

    // stop/close controls
    if (ImGui::Button("Stop Capture / Close File"))
    {
        m_captureSystem.stop();
        m_state = AppState::IDLE;
    }

    ImGui::End();
}
}