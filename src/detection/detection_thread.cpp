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
    std::shared_ptr<CaptureFrame> lastProcessedPtr = nullptr;

    while (!stoken.stop_requested())
    {
        auto frameOpt = m_captureSystem.getLatestFrame();
        
        if (!frameOpt.has_value())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        auto frame = frameOpt.value();

        if (frame->frameIndex < m_lastProcessedFrame)
        {
            m_lastProcessedFrame = 0; 
        }

        if (frame->frameIndex == m_lastProcessedFrame && frame == lastProcessedPtr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        m_lastProcessedFrame = frame->frameIndex;
        lastProcessedPtr = frame;

        auto start_time = std::chrono::steady_clock::now();
        // pass frame to onnx
        auto result = m_detector->processFrame(frame);

        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = end_time - start_time;

        // calculate fps
        float instant_fps = (elapsed.count() > .0f) ? (1.0f / elapsed.count()) : .0f;
        m_currentFps.store((m_currentFps.load() * .9f) + (instant_fps * .1f));
        
        if (result.is_ok())
        {
            // ring-buffer behavior - if full, pop the oldest and push the newest
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

}