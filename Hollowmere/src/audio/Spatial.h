#pragma once

#include "core/mathx.h"
#include <vector>
#include <cstdint>

namespace hm {

struct Listener3D {
    Vec3 position{0.0f, 1.65f, 0.0f};
    float yawDeg = 0.0f;
    float pitchDeg = 0.0f;
};

struct SpatialParams {
    float gainLeft = 1.0f;
    float gainRight = 1.0f;
    int delaySamplesLeft = 0;   // ITD delay in samples
    int delaySamplesRight = 0;
    float lowpassCutoff = 20000.0f; // Air absorption & occlusion cutoff (Hz)
    float distance = 0.0f;
    float azimuthDeg = 0.0f;   // -180 (back-left) to +180 (back-right), 0 = straight ahead
    float elevationDeg = 0.0f;
    bool isOccluded = false;
};

class Spatial {
public:
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr float SPEED_OF_SOUND = 343.0f; // m/s
    static constexpr float HEAD_RADIUS = 0.0875f;    // ~8.75 cm
    static constexpr float MAX_ITD_SEC = (HEAD_RADIUS * 2.5f) / SPEED_OF_SOUND; // ~0.64 ms

    static SpatialParams compute(const Vec3& emitterPos,
                                 const Listener3D& listener,
                                 float maxDistance = 25.0f,
                                 float rolloff = 1.0f,
                                 bool binauralMode = true,
                                 bool directOcclusion = false,
                                 bool closedDoor = false);

    // Apply 1-pole lowpass filter in-place to stereo PCM
    static void applyLowpass(std::vector<int16_t>& samples, float cutoffHz, int sampleRate = SAMPLE_RATE);
};

} // namespace hm
