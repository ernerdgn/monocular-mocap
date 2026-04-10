#pragma once
#include "capture/capture_thread.hpp"
#include "detection/detection_thread.hpp"
#include "ui/texture.hpp"
#include "ui_types.hpp"

#include <memory>
#include <vector>

namespace mocap
{
class MainUI
{
  public:
    MainUI(CaptureThread& captureSystem, DetectionThread& detectionThread, Texture& cameraTexture, int defaultCameraId);
    void render();

  private:
    ApplicationState m_appState;
    std::vector<std::unique_ptr<IPanel>> m_panels;
};
} // namespace mocap