#pragma once
#include "core/types.hpp"
#include "one_euro_filter.hpp"
#include <array>
#include <vector>
#include <deque>

namespace mocap {

class TemporalSmoother {
public:
    TemporalSmoother(float minCutoff = 1.0f, float beta = 0.05f);

    // dynamic tuning
    void setParameters(float minCutoff, float beta);

    std::vector<PoseFrame> process(const PoseFrame& frame);

    void reset();

private:
    // 55 distinct filters for the 55 smplx joints
    std::array<QuatOneEuroFilter, 55> m_jointFilters;
    
    // 3 scalar filters for the root translation (x,y,z)
    std::array<ScalarOneEuroFilter, 3> m_translFilters;

    float m_minCutoff;
    float m_beta;

    bool m_hasLastConfident = false;
    PoseFrame m_lastConfidentFrame;

    std::deque<PoseFrame> m_occlusionBuffer;
    
    const float k_confidenceThreshold = 0.5f;
    const float k_maxGapDuration = 1.0f;
};

}