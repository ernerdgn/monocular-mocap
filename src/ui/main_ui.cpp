#include "main_ui.hpp"

#include "console_panel.hpp"
#include "controls_panel.hpp"
#include "status_panel.hpp"
#include "viewport_panel.hpp"
#include "scene_panel.hpp"

namespace mocap
{

MainUI::MainUI(CaptureThread& captureSystem, DetectionThread& detectionThread, FittingThread& fittingThread, SmoothingThread& smoothingThread, Texture& cameraTexture, int defaultCameraId)
    : m_appState(AppState::IDLE)
{
    // register ui panels
    m_panels.push_back(std::make_unique<StatusPanel>(captureSystem));
    m_panels.push_back(std::make_unique<ControlsPanel>(captureSystem, detectionThread, smoothingThread, defaultCameraId));
    m_panels.push_back(std::make_unique<ViewportPanel>(captureSystem, detectionThread, cameraTexture));
    m_panels.push_back(std::make_unique<ConsolePanel>());

    // init and register 3d scene
    auto scenePanel = std::make_unique<ScenePanel>(fittingThread);
    scenePanel->initialize(); // build opengl buffers
    m_panels.push_back(std::move(scenePanel));
}

void MainUI::render()
{
    for (auto& panel : m_panels)
    {
        panel->render(m_appState);
    }
}
}