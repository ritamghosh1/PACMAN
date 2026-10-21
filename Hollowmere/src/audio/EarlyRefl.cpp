#include "EarlyRefl.h"
#include <cmath>
#include <algorithm>

namespace hm {

EarlyReflections::EarlyReflections() {
    configureRoom(6.0f, 3.2f, 8.0f);
}

void EarlyReflections::configureRoom(float width, float height, float depth, float wallAbsorption) {
    m_taps.clear();
    float speed = 343.0f; // m/s
    float reflGain = 1.0f - wallAbsorption;

    // First order reflections: left wall, right wall, ceiling, floor, front wall, back wall
    // Second order reflections: corner paths
    float paths[12] = {
        width * 0.9f,
        width * 1.1f,
        height * 0.8f,
        height * 1.2f,
        depth * 0.7f,
        depth * 1.3f,
        std::sqrt(width * width + depth * depth) * 0.65f,
        std::sqrt(width * width + height * height) * 0.70f,
        std::sqrt(depth * depth + height * height) * 0.75f,
        (width + depth) * 0.6f,
        (width + height) * 0.65f,
        (depth + height) * 0.7f
    };

    for (int i = 0; i < 12; ++i) {
        float delaySec = paths[i] / speed;
        int delaySamples = static_cast<int>(delaySec * SAMPLE_RATE);
        delaySamples = clamp(delaySamples, 80, 8000); // 1.8ms to 180ms

        float atten = (1.0f / (1.0f + paths[i] * 0.18f)) * reflGain;
        if (i >= 6) atten *= reflGain; // 2nd order bounce extra loss

        // Spatial panning of reflections (alternating left/center/right)
        float pan = (i % 3 == 0) ? -0.7f : (i % 3 == 1) ? 0.7f : 0.0f;
        float gl = atten * std::min(1.0f, 1.0f - pan);
        float gr = atten * std::min(1.0f, 1.0f + pan);

        m_taps.push_back({delaySamples, gl, gr});
    }
}

void EarlyReflections::process(const std::vector<int16_t>& monoIn,
                              std::vector<int16_t>& stereoOut,
                              float wetLevel) {
    size_t inLen = monoIn.size();
    if (inLen == 0) return;

    size_t maxDelay = 0;
    for (const auto& tap : m_taps) {
        if (static_cast<size_t>(tap.delaySamples) > maxDelay) {
            maxDelay = tap.delaySamples;
        }
    }

    size_t outLen = inLen + maxDelay;
    std::vector<float> accumL(outLen, 0.0f);
    std::vector<float> accumR(outLen, 0.0f);

    // Direct path
    for (size_t i = 0; i < inLen; ++i) {
        float s = static_cast<float>(monoIn[i]);
        accumL[i] += s;
        accumR[i] += s;
    }

    // Taps
    for (const auto& tap : m_taps) {
        for (size_t i = 0; i < inLen; ++i) {
            size_t idx = i + tap.delaySamples;
            float s = static_cast<float>(monoIn[i]);
            accumL[idx] += s * tap.gainLeft * wetLevel;
            accumR[idx] += s * tap.gainRight * wetLevel;
        }
    }

    stereoOut.resize(outLen * 2);
    for (size_t i = 0; i < outLen; ++i) {
        stereoOut[i * 2 + 0] = static_cast<int16_t>(clamp(accumL[i], -32767.0f, 32767.0f));
        stereoOut[i * 2 + 1] = static_cast<int16_t>(clamp(accumR[i], -32767.0f, 32767.0f));
    }
}

} // namespace hm
