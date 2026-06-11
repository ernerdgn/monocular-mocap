#include "smoothing_thread.hpp"
#include <chrono>

namespace mocap {

SmoothingThread::SmoothingThread(FittingThread& fittingThread,
                                 std::shared_ptr<ConcurrentQueue<PoseFrame>> outputQueue)
    : m_fittingThread(fittingThread), m_outputQueue(outputQueue)
{
    m_thread = std::jthread([this](std::stop_token stoken) { processLoop(stoken); });
}

SmoothingThread::~SmoothingThread() 
{
    if (m_thread.joinable()) {
        m_thread.request_stop();
        m_thread.join();
    }
}

void SmoothingThread::setParameters(float minCutoff, float beta)
{
    m_minCutoff.store(minCutoff);
    m_beta.store(beta);
}

void SmoothingThread::setBypass(bool bypass)
{
    m_bypass.store(bypass);
}

bool SmoothingThread::isOccluded() const
{
    return m_isOccluded.load();
}

void SmoothingThread::processLoop(std::stop_token stoken)
{
    uint64_t lastProcessedIndex = 0xFFFFFFFFFFFFFFFF;

    while (!stoken.stop_requested())
    {
        auto frame_opt = m_fittingThread.getLatestPose();
        
        if (!frame_opt.has_value() || frame_opt.value().frameIndex == lastProcessedIndex)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }

        PoseFrame frame = frame_opt.value();
        lastProcessedIndex = frame.frameIndex;

        if (m_bypass.load())
        {
            m_isOccluded.store(false);
            m_outputQueue->push(frame);
        }

        else
        {
            m_smoother.setParameters(m_minCutoff.load(), m_beta.load());
            m_isOccluded.store(frame.overall_confidence < 0.5f);

            std::vector<PoseFrame> processed_frames = m_smoother.process(frame);
            for (const auto& sf : processed_frames)
            {
                m_outputQueue->push(sf);
            }
        }
    }
}

}