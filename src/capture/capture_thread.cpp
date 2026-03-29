#include "capture_thread.hpp"

#include "core/logger.hpp"

#include <chrono>

namespace mocap
{

CaptureThread::~CaptureThread()
{
    stop();
}

Result<void> CaptureThread::start(std::unique_ptr<ICaptureSource> source)
{
    stop(); // ensure any existing thread is completely stopped

    m_source = std::move(source);
    auto res = m_source->open();
    if (!res.is_ok())
    {
        m_source.reset();
        return res;
    }

    // create a fresh queue for the new session
    m_pool = FramePool::create(3);
    m_frameQueue = std::make_unique<ConcurrentQueue<std::shared_ptr<CaptureFrame>>>(2);
    m_isRunning  = true;

    m_worker = std::jthread(&CaptureThread::threadLoop, this);
    MOCAP_INFO("CaptureThread started.");

    return Result<void>::ok();
}

void CaptureThread::stop()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        if (m_frameQueue)
            m_frameQueue->close(); // unblock the worker

        if (m_worker.joinable())
        {
            m_worker.join();
        }
    }
    if (m_source)
    {
        m_source->close();
        m_source.reset();
    }
}

void CaptureThread::threadLoop()
{
    double targetFps = m_source->getFPS();
    // default to 30fps if the source cant report it
    double targetFrameTimeMs = (targetFps > 0.0) ? (1000.0 / targetFps) : 33.3;

    while (m_isRunning)
    {
        auto start_time = std::chrono::steady_clock::now();

        auto framePtr = m_pool->acquire();
        auto res = m_source->readFrame(framePtr->image);
        if (!res.is_ok())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!m_frameQueue->try_push(framePtr))
        {
            m_frameQueue->try_pop(); 
            m_frameQueue->try_push(framePtr);
        }

        // Throttle to match the video's FPS
        auto end_time                                     = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

        if (elapsed.count() < targetFrameTimeMs)
        {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(targetFrameTimeMs - elapsed.count()));
        }
    }
}

std::optional<std::shared_ptr<CaptureFrame>> CaptureThread::getLatestFrame()
{
    if (!m_frameQueue) return std::nullopt;

    std::optional<std::shared_ptr<CaptureFrame>> latest;
    std::optional<std::shared_ptr<CaptureFrame>> current;
    while ((current = m_frameQueue->try_pop()).has_value())
    {
        latest = current;
    }
    return latest;
}

}