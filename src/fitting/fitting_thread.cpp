#include "fitting_thread.hpp"
#include "core/logger.hpp"
#include <chrono>

namespace mocap {

FittingThread::FittingThread(std::shared_ptr<PoseOptimizer> optimizer, DetectionThread& detection_source)
    : m_optimizer(std::move(optimizer)), m_detectionSource(detection_source) 
{
}

FittingThread::~FittingThread() 
{
    stop();
}

void FittingThread::start() 
{
    if (m_isRunning) return;
    m_isRunning = true;
    m_worker = std::thread(&FittingThread::processLoop, this);
    MOCAP_INFO("Fitting Thread started.");
}

void FittingThread::stop() 
{
    if (!m_isRunning) return;
    m_isRunning = false;
    if (m_worker.joinable()) {
        m_worker.join();
    }
    MOCAP_INFO("Fitting Thread stopped.");
}

std::optional<PoseFrame> FittingThread::getLatestPose() 
{
    std::lock_guard<std::mutex> lock(m_poseMutex);
    return m_latestPose;
}

void FittingThread::processLoop() 
{
    uint64_t last_processed_frame = 0;
    bool first_frame = true;

    while (m_isRunning) 
    {
        // poll ai thread
        auto det_opt = m_detectionSource.getLatestResult();
        
        if (det_opt.has_value() && (first_frame || det_opt.value().frameIndex != last_processed_frame)) 
        {
            last_processed_frame = det_opt.value().frameIndex;
            first_frame = false;

            FlowResult dummy_flow;
            PoseFrame new_pose = m_optimizer->optimizeFrame(det_opt.value(), dummy_flow, m_currentMode.load());

            // debug flag
            static int thread_debug_count = 0;
            if (thread_debug_count < 5) {
                MOCAP_INFO("DEBUG [Thread]: Processed Frame {}. Output Skeleton has {} joints.", 
                    det_opt.value().frameIndex, 
                    new_pose.body_joints.size());
                thread_debug_count++;
            }

            // store result
            {
                std::lock_guard<std::mutex> lock(m_poseMutex);
                m_latestPose = new_pose;
            }
        } 
        else 
        {
            // is there any data
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
}

}