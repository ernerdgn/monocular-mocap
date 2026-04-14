#pragma once
#include "core/types.hpp"
#include <string>

namespace mocap {
struct ApplicationState {
    AppState currentState = AppState::IDLE;

    float currentMotionMagnitude = 0.0f;

    //TODO:
    // std::shared_ptr<CaptureSession> currentSession;
    // ExportSettings exportSettings;
    // float speedMultiplier = 1.0f;
    // CameraResolutionTier currentTier;
    // std::string lastError;
};
}