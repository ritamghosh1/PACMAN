#pragma once

#include <vector>
#include <cstdint>

namespace hm {

class Synth {
public:
    static constexpr int SAMPLE_RATE = 44100;

    // Generates 16-bit mono PCM sample buffers
    static std::vector<int16_t> generateFootstep(int variant = 0);
    static std::vector<int16_t> generateDoorCreak();
    static std::vector<int16_t> generateDoorSlam();
    static std::vector<int16_t> generateHeartbeat(float bpm = 80.0f);
    static std::vector<int16_t> generateBreathing(bool panting = false);
    static std::vector<int16_t> generateAmbientDrone(float durationSeconds = 4.0f);
    static std::vector<int16_t> generateGhostWail(float durationSeconds = 3.0f);
    static std::vector<int16_t> generateLanternClick();
    static std::vector<int16_t> generatePickupChime();
    static std::vector<int16_t> generateJumpscareStinger();
};

} // namespace hm
