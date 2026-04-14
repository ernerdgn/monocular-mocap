#include "controls_panel.hpp"
#include "capture/camera_source.hpp"
#include "capture/file_source.hpp"
#include <imgui.h>
#include <nfd.hpp>
#include <memory>

namespace mocap {

ControlsPanel::ControlsPanel(CaptureThread& captureSystem, DetectionThread& detectionSystem, int defaultCameraId)
    : m_captureSystem(captureSystem), m_detectionSystem(detectionSystem), m_selectedCameraId(defaultCameraId)
    {
        m_availableCameras = DeviceEnumerator::getAvailableCameras();
    
        // ensure selected id is valid
        if (!m_availableCameras.empty() && m_selectedCameraId < 0)
        {
            m_selectedCameraId = m_availableCameras.front().id;
        }
    }

void ControlsPanel::render(ApplicationState& state)
{
    ImGui::Begin("Controls");
    
    // app state
    const char* stateText = "IDLE";
    if (state.currentState == AppState::CAPTURING) stateText = "CAPTURING (Live Camera)";
    else if (state.currentState == AppState::REVIEWING) stateText = "REVIEWING (Video File)";
    
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
            state.currentState = AppState::CAPTURING;
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
                state.currentState = AppState::REVIEWING;
            }
            NFD::FreePath(outPath);
        }
    }

    ImGui::Separator();

    // stop/close controls
    if (ImGui::Button("Stop Capture / Close File"))
    {
        m_captureSystem.stop();
        state.currentState = (state.currentState == AppState::CAPTURING) ? AppState::REVIEWING : AppState::IDLE;
    }

    // ai telemetry dashboard
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextDisabled("AI Telemetry");
    ImGui::Spacing();

    // display detection fps
    ImGui::Text("AI Speed: %.1f FPS", m_detectionSystem.getFPS());

    auto maybe_result = m_detectionSystem.getLatestResult();
    if (maybe_result.has_value() && maybe_result.value().overallConfidence > 0.0f)
    {
        const auto& result = maybe_result.value();
        
        // display overall confidence
        ImGui::Text("Overall Confidence: %.0f%%", result.overallConfidence * 100.0f);
        ImGui::Spacing();

        // display per-joint table
        if (ImGui::BeginTable("JointTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableSetupColumn("Joint");
            ImGui::TableSetupColumn("Confidence");
            ImGui::TableHeadersRow();

            // 17 standard joints map according to BodyJoint enum
            const char* jointNames[17] = {
                "Nose", "L Eye", "R Eye", "L Ear", "R Ear", 
                "L Shoulder", "R Shoulder", "L Elbow", "R Elbow", 
                "L Wrist", "R Wrist", "L Hip", "R Hip", 
                "L Knee", "R Knee", "L Ankle", "R Ankle"
            };
            
            for (int i = 0; i < 17; ++i) 
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", jointNames[i]);
                
                ImGui::TableNextColumn();
                float conf = result.bodyJoints[i].confidence;
                
                // i like rainbows
                if (conf > 0.7f) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.2f", conf);
                else if (conf > 0.4f) ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.2f", conf);
                else ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%.2f", conf);
            }

            ImGui::EndTable();
        }
    }

    else
    {
        // no-detection clear state
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No person detected in frame.");
    }

    // motion indicator
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextDisabled("Motion Tracking");
    ImGui::Spacing();
    
    ImGui::Text("Magnitude: %.3f", state.currentMotionMagnitude);

    // dynamic coloring: green if motion > .01 (threshold), otherwise default/white
    ImVec4 barColor = state.currentMotionMagnitude > 0.01f ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
    // draw progress bar 
    // capping the visual limit at .1f so small movements look SIGNIFICANT
    float displayValue = std::min(state.currentMotionMagnitude * 10.0f, 1.0f); 
    ImGui::ProgressBar(displayValue, ImVec2(-1.0f, 0.0f), "");
    ImGui::PopStyleColor();

    ImGui::End();
}
}