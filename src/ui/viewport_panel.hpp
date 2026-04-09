#pragma once
#include "capture/capture_thread.hpp"
#include "detection/detection_thread.hpp"
#include "ui/texture.hpp"
#include "ui_types.hpp"
#include <imgui.h>

namespace mocap
{
class ViewportPanel : public IPanel
{
  public:
    ViewportPanel(CaptureThread& captureSystem, DetectionThread& detectionSystem, Texture& texture, AppState& state);
    void render() override;

  private:
    CaptureThread& m_captureSystem;
    DetectionThread& m_detectionSystem;
    Texture& m_texture;
    AppState& m_state;

    DetectionResult m_lastResult; 
    bool m_hasResult = false;

    ImU32 getConfidenceColor(float confidence) const;
};
} // namespace mocap