#pragma once
#include "capture/capture_thread.hpp"
#include "detection/detection_thread.hpp"
#include "ui_types.hpp"
#include "capture/device_enumerator.hpp"

namespace mocap
{
class ControlsPanel : public IPanel
{
  public:
    ControlsPanel(CaptureThread& captureSystem, DetectionThread& detectionSystem, AppState& state, int defaultCameraId);
    void render() override;
    
    std::vector<CameraDevice> m_availableCameras;

  private:
    CaptureThread& m_captureSystem;
    DetectionThread& m_detectionSystem;
    AppState& m_state;
    int m_selectedCameraId;
};
}