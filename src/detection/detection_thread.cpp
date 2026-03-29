#include "detection_thread.hpp"
#include "core/logger.hpp"
#include <chrono>

namespace mocap {

DetectionThread::DetectionThread(CaptureThread& captureSystem, std::unique_ptr<IDetector> detector)
    : m_captureSystem(captureSystem), m_detector(std::move(detector)) {}

DetectionThread::~DetectionThread() { stop(); }

void DetectionThread::start()
{
    if (m_worker.joinable()) return; // already running
    
    // queue size 2 ensures ui always gets new ai result
    m_resultQueue = std::make_unique<ConcurrentQueue<DetectionResult>>(2);
    m_worker = std::jthread([this](std::stop_token stoken) { threadLoop(stoken); });
    MOCAP_INFO("Detection thread started.");
}

void DetectionThread::stop()
{
    if (m_worker.joinable())
    {
        m_worker.request_stop();
        m_worker.join();
        MOCAP_INFO("Detection thread stopped.");
    }
}

std::optional<DetectionResult> DetectionThread::getLatestResult()
{
    if (!m_resultQueue) return std::nullopt;

    std::optional<DetectionResult> latest;
    std::optional<DetectionResult> current;
    // drain queue to get new result
    while ((current = m_resultQueue->try_pop()).has_value())
    {
        latest = current;
    }
    return latest;
}

void DetectionThread::threadLoop(std::stop_token stoken)
{
    while (!stoken.stop_requested())
    {
        auto frameOpt = m_captureSystem.getLatestFrame();
        
        // if no frame ready, or already processed, take a coffee break
        if (!frameOpt.has_value() || frameOpt.value()->frameIndex <= m_lastProcessedFrame)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        auto frame = frameOpt.value();
        m_lastProcessedFrame = frame->frameIndex;

        // pass frame to the onnx runtime
        auto result = m_detector->processFrame(frame);
        
        if (result.is_ok())
        {
            if (!m_resultQueue->try_push(result.value()))
            {
                m_resultQueue->try_pop(); 
                m_resultQueue->try_push(result.value());
            }
        }
        else
        {
            MOCAP_WARN("Detection failed: {}", result.error());
        }
    }
}

} // namespace mocap