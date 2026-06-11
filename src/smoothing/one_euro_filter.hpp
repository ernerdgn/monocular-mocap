#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cmath>

namespace mocap {

class ScalarOneEuroFilter {
public:
    ScalarOneEuroFilter(float minCutoff = 1.0f, float beta = 0.05f, float dCutoff = 1.0f)
        : m_minCutoff(minCutoff), m_beta(beta), m_dCutoff(dCutoff) {}

    float filter(float value, float timestamp)
    {
        if (m_firstTime) {
            m_firstTime = false;
            m_prevValue = value;
            m_prevTimestamp = timestamp;
            return value;
        }

        float dt = timestamp - m_prevTimestamp;
        if (dt <= 0.0f) return value; // Safety against identical timestamps

        // calculate derivative
        float dx = (value - m_prevValue) / dt;
        
        // filter the derivative to smooth velocity spikes
        float alpha_d = computeAlpha(m_dCutoff, dt);
        float edx = m_prevDx + alpha_d * (dx - m_prevDx);
        m_prevDx = edx;

        // calculate the dynamic cutoff frequency based on speed
        float cutoff = m_minCutoff + m_beta * std::abs(edx);

        // filter the actual value
        float alpha_v = computeAlpha(cutoff, dt);
        float filtered_value = m_prevValue + alpha_v * (value - m_prevValue);

        m_prevValue = filtered_value;
        m_prevTimestamp = timestamp;

        return filtered_value;
    }

    void reset()
    { 
        m_firstTime = true; 
        m_prevDx = 0.0f; 
    }

private:
    float computeAlpha(float cutoff, float dt) const
    {
        float tau = 1.0f / (2.0f * 3.14159265359f * cutoff);
        return dt / (dt + tau);
    }

    float m_minCutoff;
    float m_beta;
    float m_dCutoff; // 1.0hz

    bool m_firstTime = true;
    float m_prevValue = 0.0f;
    float m_prevDx = 0.0f;
    float m_prevTimestamp = 0.0f;
};

class QuatOneEuroFilter {
public:
    QuatOneEuroFilter(float minCutoff = 1.0f, float beta = 0.05f)
    {
        for (int i = 0; i < 4; ++i)
        {
            m_filters[i] = ScalarOneEuroFilter(minCutoff, beta, 1.0f);
        }
    }

    void setParameters(float minCutoff, float beta)
    {
        for (int i = 0; i < 4; ++i)
        {
            m_filters[i] = ScalarOneEuroFilter(minCutoff, beta, 1.0f);
        }
    }

    glm::quat filter(const glm::quat& q, float timestamp)
    {
        glm::quat result;
        result.x = m_filters[0].filter(q.x, timestamp);
        result.y = m_filters[1].filter(q.y, timestamp);
        result.z = m_filters[2].filter(q.z, timestamp);
        result.w = m_filters[3].filter(q.w, timestamp);
        
        // normalize
        return glm::normalize(result);
    }

    void reset()
    {
        for (int i = 0; i < 4; ++i) m_filters[i].reset();
    }

private:
    ScalarOneEuroFilter m_filters[4];
};

}