#pragma once
#include "capture/capture_thread.hpp"
#include "detection/detection_thread.hpp"
#include "ui_types.hpp"
#include "application_state.hpp"
#include "capture/device_enumerator.hpp"

namespace mocap
{
class ControlsPanel : public IPanel
{
  public:
    ControlsPanel(CaptureThread& captureSystem, DetectionThread& detectionSystem, int defaultCameraId);
    void render(ApplicationState& state) override;
    
    std::vector<CameraDevice> m_availableCameras;

  private:
    CaptureThread& m_captureSystem;
    DetectionThread& m_detectionSystem;
    int m_selectedCameraId;
};
}