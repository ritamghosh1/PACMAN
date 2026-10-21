#include "Sanity.h"
#include <algorithm>

namespace hm {

SanitySystem::SanitySystem() = default;

void SanitySystem::update(float dt,
                          bool inDarkness,
                          bool nearGhost,
                          bool inSafeRoom,
                          bool isListening,
                          bool isHumming) {
    if (inSafeRoom) {
        m_sanity = std::min(100.0f, m_sanity + 4.5f * dt);
        return;
    }

    float delta = 0.0f;
    if (inDarkness) delta -= 0.85f * dt;
    else delta += 0.40f * dt; // Light calms

    if (nearGhost) delta -= 2.6f * dt;
    if (isListening) delta -= 0.50f * dt;
    if (isHumming) delta += 1.8f * dt;

    m_sanity = clamp(m_sanity + delta, 0.0f, 100.0f);
}

float SanitySystem::desaturationAmount() const {
    if (m_sanity > 60.0f) return 0.0f;
    return (60.0f - m_sanity) / 60.0f * 0.70f;
}

float SanitySystem::chromaticAberration() const {
    if (m_sanity > 50.0f) return 0.002f;
    return 0.002f + (50.0f - m_sanity) / 50.0f * 0.015f;
}

} // namespace hm
