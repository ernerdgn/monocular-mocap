#pragma once
#include "capture/capture_thread.hpp"
#include "ui_types.hpp"

namespace mocap
{

class StatusPanel : public IPanel
{
  public:
    StatusPanel(CaptureThread& captureSystem, const AppState& state);
    void render() override;

  private:
    CaptureThread& m_captureSystem;
    const AppState& m_state;
};

} // namespace mocap