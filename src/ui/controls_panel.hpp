#pragma once
#include "ui_types.hpp"
#include "capture/capture_thread.hpp"

namespace mocap {
    class ControlsPanel : public IPanel
    {
        public:
        ControlsPanel(CaptureThread& captureSystem, AppState& state, int defaultCameraId);
        void render() override;
        
        private:
        CaptureThread& m_captureSystem;
        AppState& m_state;
        int m_selectedCameraId;
    };
}