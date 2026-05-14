#pragma once
#include "fitting/pose_optimizer.hpp"
#include "detection/detection_thread.hpp"
#include "core/logger.hpp"

#include <thread>
#include <atomic>
#include <mutex>
#include <optional>
#include <memory>

namespace mocap {

class FittingThread 
{
public:
    FittingThread(std::shared_ptr<PoseOptimizer> optimizer, DetectionThread& detection_source);
    ~FittingThread();

    void start();
    void stop();

    std::optional<PoseFrame> getLatestPose();

    void setMode(FittingMode mode)
    { 
        m_currentMode.store(mode); 
        MOCAP_INFO("FittingThread switched to {} mode", mode == FittingMode::LIVE ? "LIVE" : "EXPORT");
    }

private:
    std::shared_ptr<PoseOptimizer> m_optimizer;
    DetectionThread& m_detectionSource;

    std::thread m_worker;
    std::atomic<bool> m_isRunning{false};

    std::mutex m_poseMutex;
    std::optional<PoseFrame> m_latestPose;

    std::atomic<FittingMode> m_currentMode{FittingMode::LIVE};

    void processLoop();
};

}