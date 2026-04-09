#include "main_ui.hpp"

#include "console_panel.hpp"
#include "controls_panel.hpp"
#include "status_panel.hpp"
#include "viewport_panel.hpp"

namespace mocap
{

MainUI::MainUI(CaptureThread& captureSystem, DetectionThread& detectionThread, Texture& cameraTexture, int defaultCameraId)
    : m_currentState(AppState::IDLE)
{
    // register ui panels
    m_panels.push_back(std::make_unique<StatusPanel>(captureSystem, m_currentState));
    m_panels.push_back(std::make_unique<ControlsPanel>(captureSystem, detectionThread, m_currentState, defaultCameraId));
    m_panels.push_back(std::make_unique<ViewportPanel>(captureSystem, detectionThread, cameraTexture, m_currentState));
    m_panels.push_back(std::make_unique<ConsolePanel>());
}

void MainUI::render()
{
    for (auto& panel : m_panels)
    {
        panel->render();
    }
}
}