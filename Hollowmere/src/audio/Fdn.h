#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace hm {

// 8-delay-line Feedback Delay Network (FDN) Reverb
class FdnReverb {
public:
    static constexpr int NUM_LINES = 8;
    static constexpr int SAMPLE_RATE = 44100;

    struct RoomAcoustics {
        float rt60 = 1.8f;        // Reverb decay time in seconds
        float damping = 0.35f;    // High frequency absorption (0.0 = bright, 0.8 = dark)
        float wetLevel = 0.28f;   // Wet mix fraction
        float roomScale = 1.0f;   // Geometry scalar
    };

    FdnReverb() {
        // Prime delay lengths in samples (~25ms to 68ms)
        const int baseDelays[NUM_LINES] = {1117, 1361, 1601, 1867, 2129, 2411, 2677, 2953};
        for (int i = 0; i < NUM_LINES; ++i) {
            m_delays[i] = baseDelays[i];
            m_buffers[i].resize(8192, 0.0f);
            m_indices[i] = 0;
            m_dampState[i] = 0.0f;
        }
        setRoom(RoomAcoustics{});
    }

    void setRoom(const RoomAcoustics& room) {
        m_room = room;
        // Compute feedback loop gain per line based on RT60:
        // gain = 10 ^ (-3 * delaySec / RT60)
        for (int i = 0; i < NUM_LINES; ++i) {
            float delaySec = static_cast<float>(m_delays[i]) / static_cast<float>(SAMPLE_RATE);
            m_feedbackGains[i] = std::pow(10.0f, -3.0f * delaySec / std::max(0.1f, m_room.rt60));
        }
    }

    // Process a single mono sample through the FDN, return stereo (outL, outR)
    void processSample(float in, float& outL, float& outR) {
        float delayOutputs[NUM_LINES];

        // Read from delay lines
        for (int i = 0; i < NUM_LINES; ++i) {
            int readIdx = m_indices[i] - m_delays[i];
            if (readIdx < 0) readIdx += static_cast<int>(m_buffers[i].size());
            delayOutputs[i] = m_buffers[i][readIdx];
        }

        // Householder reflection matrix: y = x - (2/N)*sum(x)
        float sum = 0.0f;
        for (int i = 0; i < NUM_LINES; ++i) {
            sum += delayOutputs[i];
        }
        float factor = (2.0f / static_cast<float>(NUM_LINES)) * sum;

        float wetL = 0.0f;
        float wetR = 0.0f;

        // Feedback loop with damping and input injection
        for (int i = 0; i < NUM_LINES; ++i) {
            float reflected = delayOutputs[i] - factor;

            // 1-pole lowpass damping filter
            m_dampState[i] = reflected * (1.0f - m_room.damping) + m_dampState[i] * m_room.damping;
            float feedbackVal = m_dampState[i] * m_feedbackGains[i];

            // Inject input
            m_buffers[i][m_indices[i]] = in + feedbackVal;
            m_indices[i] = (m_indices[i] + 1) % m_buffers[i].size();

            // Distribute outputs to left and right channels with alternating phase
            if (i % 2 == 0) {
                wetL += delayOutputs[i] * (i < 4 ? 0.35f : -0.35f);
            } else {
                wetR += delayOutputs[i] * (i < 4 ? 0.35f : -0.35f);
            }
        }

        outL = in * (1.0f - m_room.wetLevel) + wetL * m_room.wetLevel;
        outR = in * (1.0f - m_room.wetLevel) + wetR * m_room.wetLevel;
    }

    void reset() {
        for (int i = 0; i < NUM_LINES; ++i) {
            std::fill(m_buffers[i].begin(), m_buffers[i].end(), 0.0f);
            m_indices[i] = 0;
            m_dampState[i] = 0.0f;
        }
    }

private:
    RoomAcoustics m_room;
    int m_delays[NUM_LINES];
    float m_feedbackGains[NUM_LINES];
    float m_dampState[NUM_LINES];
    std::vector<float> m_buffers[NUM_LINES];
    int m_indices[NUM_LINES];
};

} // namespace hm
