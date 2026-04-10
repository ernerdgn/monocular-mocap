#pragma once
#include "core/types.hpp"
#include <string>

namespace mocap {
struct ApplicationState {
    AppState currentState = AppState::IDLE;

    //TODO:
    // std::shared_ptr<CaptureSession> currentSession;
    // ExportSettings exportSettings;
    // float speedMultiplier = 1.0f;
    // CameraResolutionTier currentTier;
    // std::string lastError;
};
}