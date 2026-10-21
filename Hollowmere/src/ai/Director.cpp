#include "Director.h"
#include <cstdlib>
#include <algorithm>

namespace hm {

Director::Director() {
}

void Director::update(float dt, float playerSanity, float nearestGhostDist, bool playerInSafeRoom) {
    if (m_breatherTimer > 0.0f) {
        m_breatherTimer -= dt;
        if (m_breatherTimer < 0.0f) m_breatherTimer = 0.0f;
    }

    if (m_scareCooldownTimer > 0.0f) {
        m_scareCooldownTimer -= dt;
        if (m_scareCooldownTimer < 0.0f) m_scareCooldownTimer = 0.0f;
    }

    m_ambientScareCheckTimer += dt;

    if (playerInSafeRoom) {
        m_intensity += (0.05f - m_intensity) * std::min(1.0f, dt * 0.8f);
        return;
    }

    if (m_breatherTimer > 0.0f) {
        m_intensity += (0.15f - m_intensity) * std::min(1.0f, dt * 0.6f);
        return;
    }

    // Compute target threat intensity
    float ghostThreat = 0.0f;
    if (nearestGhostDist < 18.0f) {
        ghostThreat = clamp(1.0f - (nearestGhostDist / 18.0f), 0.0f, 1.0f);
    }

    float sanityThreat = clamp((100.0f - playerSanity) / 100.0f, 0.0f, 1.0f) * 0.45f;
    float targetIntensity = std::max(ghostThreat, sanityThreat) * m_difficultyScalar;

    // Smooth intensity curve
    m_intensity += (targetIntensity - m_intensity) * std::min(1.0f, dt * 1.2f);
}

bool Director::canTriggerHardScare() const {
    if (m_hardScaresCount >= MAX_HARD_SCARES) return false;
    if (m_scareCooldownTimer > 0.0f) return false;
    if (m_breatherTimer > 0.0f) return false;
    return true;
}

void Director::recordHardScare() {
    m_hardScaresCount++;
    m_scareCooldownTimer = 50.0f; // Minimum 50s cooldown between hard scares
}

ScareType Director::checkPeriodicScare() {
    if (m_ambientScareCheckTimer < 25.0f) return ScareType::None;
    m_ambientScareCheckTimer = 0.0f;

    if (m_breatherTimer > 0.0f) return ScareType::None;

    // Ambient soft scare library
    int r = std::rand() % 4;
    switch (r) {
        case 0: return ScareType::SoftWhisper;
        case 1: return ScareType::SoftDoorCreak;
        case 2: return ScareType::SoftLightFlicker;
        default: return ScareType::None;
    }
}

void Director::registerPlayerDeath() {
    m_consecutiveDeaths++;
    if (m_consecutiveDeaths >= 3) {
        m_difficultyScalar = 0.85f; // Reduce AI pressure by 15%
    }
    triggerBreather(35.0f);
}

void Director::triggerBreather(float durationSec) {
    m_breatherTimer = durationSec;
    m_scareCooldownTimer = std::max(m_scareCooldownTimer, durationSec);
}

} // namespace hm
