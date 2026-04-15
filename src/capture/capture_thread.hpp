#pragma once
#include "capture_source.hpp"
#include "core/concurrent_queue.hpp"
#include "frame_pool.hpp"
#include <atomic>
#include <memory>
#include <optional>
#include <thread>

namespace mocap
{

class CaptureThread
{
  public:
    CaptureThread() : m_isRunning(false)
    {
    }
    ~CaptureThread();

    Result<void> start(std::unique_ptr<ICaptureSource> source);
    void stop();

    std::optional<std::shared_ptr<CaptureFrame>> getLatestFrame();

    bool isRunning() const
    {
        return m_isRunning;
    }
    ICaptureSource* getSource() const
    {
        return m_source.get();
    }

  private:
    void threadLoop();

    std::unique_ptr<ICaptureSource> m_source;
    std::jthread m_worker;
    std::atomic<bool> m_isRunning;
    std::shared_ptr<FramePool> m_pool;
    std::unique_ptr<ConcurrentQueue<std::shared_ptr<CaptureFrame>>> m_frameQueue;
    std::optional<std::shared_ptr<CaptureFrame>> m_lastFrameCache;
    //std::unique_ptr<ConcurrentQueue<cv::Mat>> m_frameQueue;
};

} // namespace mocap