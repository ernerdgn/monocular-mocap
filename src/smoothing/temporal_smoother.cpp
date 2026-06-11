#include "temporal_smoother.hpp"
#include "core/logger.hpp"

namespace mocap {

TemporalSmoother::TemporalSmoother(float minCutoff, float beta)
    : m_minCutoff(minCutoff), m_beta(beta)
{
    setParameters(minCutoff, beta);
}

void TemporalSmoother::setParameters(float minCutoff, float beta)
{
    m_minCutoff = minCutoff;
    m_beta = beta;

    for (auto& filter : m_jointFilters)
    {
        filter.setParameters(minCutoff, beta);
    }
    
    for (int i = 0; i < 3; ++i)
    {
        m_translFilters[i] = ScalarOneEuroFilter(minCutoff, beta, 1.0f);
    }
}

std::vector<PoseFrame> TemporalSmoother::process(const PoseFrame& frame)
{
    std::vector<PoseFrame> output_frames;

    if (!m_hasLastConfident) {
        if (frame.overall_confidence >= k_confidenceThreshold)
        {
            PoseFrame smoothed = frame;
            smoothed.root_translation.x = m_translFilters[0].filter(frame.root_translation.x, frame.timestamp);
            smoothed.root_translation.y = m_translFilters[1].filter(frame.root_translation.y, frame.timestamp);
            smoothed.root_translation.z = m_translFilters[2].filter(frame.root_translation.z, frame.timestamp);
            for (size_t i = 0; i < frame.body_joints.size(); ++i)
            {
                smoothed.body_joints[i].rotation = m_jointFilters[i].filter(frame.body_joints[i].rotation, frame.timestamp);
            }
            m_lastConfidentFrame = smoothed;
            m_hasLastConfident = true;
            output_frames.push_back(smoothed);
        }
        else
        {
            MOCAP_WARN("Dropping initial frame due to low confidence ({:.2f})", frame.overall_confidence);
        }
        return output_frames;
    }

    if (frame.overall_confidence < k_confidenceThreshold)
    {
        m_occlusionBuffer.push_back(frame);

        float gap_duration = frame.timestamp - m_lastConfidentFrame.timestamp;

        if (gap_duration > k_maxGapDuration) {
            MOCAP_WARN("Occlusion exceeded maximum gap threshold! Emitting frozen fallback pose.");
            
            PoseFrame fallback_frame = m_lastConfidentFrame;
            fallback_frame.timestamp = frame.timestamp;
            fallback_frame.frameIndex = frame.frameIndex;
            fallback_frame.is_interpolated = true;
            
            m_occlusionBuffer.clear();
            output_frames.push_back(fallback_frame);
        }
        
        return output_frames;
    }

    if (!m_occlusionBuffer.empty()) {
        float t_start = m_lastConfidentFrame.timestamp;
        float t_end = frame.timestamp;
        float t_range = t_end - t_start;

        MOCAP_INFO("Recovered tracking after occlusion. Interpolating {} missing frames.", m_occlusionBuffer.size());

        for (const auto& hidden_frame : m_occlusionBuffer)
        {
            float t_curr = hidden_frame.timestamp;
            float alpha = (t_range > 0.0001f) ? ((t_curr - t_start) / t_range) : 1.0f;
            alpha = glm::clamp(alpha, 0.0f, 1.0f);

            PoseFrame interp_frame = hidden_frame;
            interp_frame.is_interpolated = true;

            interp_frame.root_translation = glm::mix(m_lastConfidentFrame.root_translation, frame.root_translation, alpha);

            for (size_t i = 0; i < frame.body_joints.size(); ++i)
            {
                interp_frame.body_joints[i].rotation = glm::slerp(
                    m_lastConfidentFrame.body_joints[i].rotation, 
                    frame.body_joints[i].rotation, 
                    alpha
                );
            }

            interp_frame.root_translation.x = m_translFilters[0].filter(interp_frame.root_translation.x, interp_frame.timestamp);
            interp_frame.root_translation.y = m_translFilters[1].filter(interp_frame.root_translation.y, interp_frame.timestamp);
            interp_frame.root_translation.z = m_translFilters[2].filter(interp_frame.root_translation.z, interp_frame.timestamp);
            for (size_t i = 0; i < interp_frame.body_joints.size(); ++i)
            {
                interp_frame.body_joints[i].rotation = m_jointFilters[i].filter(interp_frame.body_joints[i].rotation, interp_frame.timestamp);
            }

            output_frames.push_back(interp_frame);
        }
        m_occlusionBuffer.clear();
    }

    PoseFrame current_smoothed = frame;
    current_smoothed.root_translation.x = m_translFilters[0].filter(frame.root_translation.x, frame.timestamp);
    current_smoothed.root_translation.y = m_translFilters[1].filter(frame.root_translation.y, frame.timestamp);
    current_smoothed.root_translation.z = m_translFilters[2].filter(frame.root_translation.z, frame.timestamp);
    for (size_t i = 0; i < frame.body_joints.size(); ++i)
    {
        current_smoothed.body_joints[i].rotation = m_jointFilters[i].filter(frame.body_joints[i].rotation, frame.timestamp);
    }

    m_lastConfidentFrame = current_smoothed;
    output_frames.push_back(current_smoothed);

    return output_frames;
}

void TemporalSmoother::reset()
{
    for (auto& filter : m_jointFilters) filter.reset();
    for (auto& filter : m_translFilters) filter.reset();
}

}