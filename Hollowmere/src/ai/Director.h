#pragma once

#include "core/mathx.h"

namespace hm {

enum class ScareType {
    None,
    SoftWhisper,
    SoftDoorCreak,
    SoftLightFlicker,
    HardJumpscare
};

class Director {
public:
    Director();

    void update(float dt, float playerSanity, float nearestGhostDist, bool playerInSafeRoom);

    float intensity() const { return m_intensity; }
    void setIntensity(float val) { m_intensity = clamp(val, 0.0f, 1.0f); }

    bool canTriggerHardScare() const;
    void recordHardScare();

    ScareType checkPeriodicScare();

    void registerPlayerDeath();
    float difficultyScalar() const { return m_difficultyScalar; }

    void triggerBreather(float durationSec = 40.0f);
    bool isBreatherActive() const { return m_breatherTimer > 0.0f; }

private:
    float m_intensity = 0.0f;
    float m_breatherTimer = 0.0f;
    float m_scareCooldownTimer = 0.0f;
    float m_ambientScareCheckTimer = 0.0f;

    int m_hardScaresCount = 0;
    static constexpr int MAX_HARD_SCARES = 8;
    int m_consecutiveDeaths = 0;
    float m_difficultyScalar = 1.0f; // 1.0 normal, 0.85 after 3 deaths
};

} // namespace hm
