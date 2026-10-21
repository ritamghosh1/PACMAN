#pragma once

#include <vector>
#include <cstdint>
#include "core/mathx.h"

namespace hm {

enum class FootstepSurface {
    Wood,
    Stone,
    Carpet,
    Metal,
    Water
};

enum class BreathState {
    Calm,
    Panting,
    Terrified,
    HoldingBreath
};

class BodyAudio {
public:
    static constexpr int SAMPLE_RATE = 44100;

    BodyAudio();

    void update(float dt, float fearLevel, float staminaPercent, bool isInjured, bool isHoldingBreath, bool isListening);

    // Footsteps with limp asymmetry (injured right leg drags/thuds heavier)
    std::vector<int16_t> generateFootstep(FootstepSurface surface, bool isRightFoot, bool isSprinting, bool isCrouching);

    // Breathing loop chunk
    std::vector<int16_t> renderBreathingChunk(float durationSec = 1.0f);

    // Dynamic heartbeat chunk
    std::vector<int16_t> renderHeartbeatChunk(float durationSec = 1.0f);

    // Tinnitus stinger (triggered after explosions, slams, or hard scares)
    void triggerTinnitus(float durationSec = 4.0f);
    std::vector<int16_t> renderTinnitusChunk(float durationSec = 1.0f);

    float currentBpm() const { return m_currentBpm; }
    BreathState breathState() const { return m_breathState; }
    bool isTinnitusActive() const { return m_tinnitusTimer > 0.0f; }

private:
    float m_fearLevel = 0.0f;
    float m_staminaPercent = 1.0f;
    bool m_isInjured = false;
    bool m_isHoldingBreath = false;
    bool m_isListening = false;

    float m_currentBpm = 68.0f;
    BreathState m_breathState = BreathState::Calm;
    float m_tinnitusTimer = 0.0f;
    float m_breathPhase = 0.0f;
    float m_heartPhase = 0.0f;
};

} // namespace hm
