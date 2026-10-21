#pragma once

#include <vector>
#include <cstdint>
#include "core/mathx.h"

namespace hm {

struct ReflectionTap {
    int delaySamples;
    float gainLeft;
    float gainRight;
};

class EarlyReflections {
public:
    static constexpr int MAX_TAPS = 12;
    static constexpr int SAMPLE_RATE = 44100;

    EarlyReflections();

    // Configure taps based on room dimensions (width, height, depth in meters)
    void configureRoom(float width, float height, float depth, float wallAbsorption = 0.3f);

    // Process mono buffer and generate stereo early reflections
    void process(const std::vector<int16_t>& monoIn,
                 std::vector<int16_t>& stereoOut,
                 float wetLevel = 0.25f);

private:
    std::vector<ReflectionTap> m_taps;
};

} // namespace hm
