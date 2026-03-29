#pragma once
#include "ui_types.hpp"
#include "capture/capture_thread.hpp"
#include "ui/texture.hpp"
#include <vector>
#include <memory>

namespace mocap {
    class MainUI {
        public:
        MainUI(CaptureThread& captureSystem, Texture& cameraTexture, int defaultCameraId);
        void render();

        private:
        AppState m_currentState;
        std::vector<std::unique_ptr<IPanel>> m_panels;
    };
}