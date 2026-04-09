#pragma once
#include "core/concurrent_queue.hpp"
#include "core/types.hpp"
#include "i_detector.hpp"
#include "capture/capture_thread.hpp"
#include <thread>
#include <memory>
#include <optional>

namespace mocap {

class DetectionThread
{
public:
    // takes reference to capture system and takes ownership of the ai detector
    DetectionThread(CaptureThread& captureSystem, std::unique_ptr<IDetector> detector);
    ~DetectionThread();

    void start();
    void stop();

    // ui will call this to get the latest 2d skeleton
    std::optional<DetectionResult> getLatestResult();
    float getFPS() const { return m_currentFps.load(); }

private:
    void threadLoop(std::stop_token stoken);

    CaptureThread& m_captureSystem;
    std::unique_ptr<IDetector> m_detector;
    std::unique_ptr<ConcurrentQueue<DetectionResult>> m_resultQueue;

    std::jthread m_worker;

    std::atomic<float> m_currentFps{0.0f};
    
    // track to dont run ai on the exact same frame again
    int m_lastProcessedFrame = -1;

    // internal cache
    std::optional<DetectionResult> m_lastResultCache;
};

}