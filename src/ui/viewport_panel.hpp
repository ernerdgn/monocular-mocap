#pragma once
#include "ui_types.hpp"
#include "capture/capture_thread.hpp"
#include "ui/texture.hpp"

namespace mocap {
    class ViewportPanel : public IPanel
    {
        public:
        ViewportPanel(CaptureThread& captureSystem, Texture& texture, AppState& state);
        void render() override;
        
        private:
        CaptureThread& m_captureSystem;
        Texture& m_texture;
        AppState& m_state;
};
}