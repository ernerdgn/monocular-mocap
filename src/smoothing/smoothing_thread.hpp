#pragma once
#include "core/concurrent_queue.hpp"
#include "core/types.hpp"
#include "temporal_smoother.hpp"
#include "fitting/fitting_thread.hpp"

#include <memory>
#include <thread>
#include <atomic>
#include <optional>

namespace mocap {

class SmoothingThread {
public:
    SmoothingThread(FittingThread& fittingThread,
        std::shared_ptr<ConcurrentQueue<PoseFrame>> outputQueue);
    ~SmoothingThread();

    // ui control setters
    void setParameters(float minCutoff, float beta);
    void setBypass(bool bypass);
    
    // ui status getter
    bool isOccluded() const;

private:
    void processLoop(std::stop_token stoken);

    FittingThread& m_fittingThread; // reference to track the state provider
    std::shared_ptr<ConcurrentQueue<PoseFrame>> m_outputQueue;
    std::jthread m_thread;
    TemporalSmoother m_smoother;

    std::atomic<float> m_minCutoff{1.0f};
    std::atomic<float> m_beta{0.05f};
    std::atomic<bool> m_bypass{false};
    std::atomic<bool> m_isOccluded{false};
};

}