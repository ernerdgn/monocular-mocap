#include "main_ui.hpp"
#include "controls_panel.hpp"
#include "viewport_panel.hpp"
#include "status_panel.hpp"
#include "console_panel.hpp"

namespace mocap {

    MainUI::MainUI(CaptureThread& captureSystem, Texture& cameraTexture, int defaultCameraId) 
        : m_currentState(AppState::IDLE)
    {
        // register ui panels
        m_panels.push_back(std::make_unique<StatusPanel>(captureSystem, m_currentState));
        m_panels.push_back(std::make_unique<ControlsPanel>(captureSystem, m_currentState, defaultCameraId));
        m_panels.push_back(std::make_unique<ViewportPanel>(captureSystem, cameraTexture, m_currentState));
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