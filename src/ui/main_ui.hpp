#pragma once
#include "capture/capture_thread.hpp"
#include "detection/detection_thread.hpp"
#include "fitting/fitting_thread.hpp"
#include "render/texture.hpp"
#include "ui_types.hpp"
#include "smoothing/smoothing_thread.hpp"

#include <memory>
#include <vector>

namespace mocap
{
class MainUI
{
  public:
    MainUI(CaptureThread& captureSystem, DetectionThread& detectionThread,
      FittingThread& fittingThread, SmoothingThread& smoothingThread,
      Texture& cameraTexture, int defaultCameraId);
    void render();

    ApplicationState& getState() { return m_appState; }

  private:
    ApplicationState m_appState;
    std::vector<std::unique_ptr<IPanel>> m_panels;
};
}