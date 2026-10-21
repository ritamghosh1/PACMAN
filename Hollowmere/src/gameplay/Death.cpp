#include "Death.h"
#include <algorithm>
#include <cmath>

namespace hm {

DeathSequence::DeathSequence() = default;

void DeathSequence::trigger(const Vec3& ghostPos) {
    m_phase = DeathPhase::Collapse;
    m_timer = 0.0f;
    m_killerPos = ghostPos;
    m_deathCount++;
    pickWakeWhisper();
}

void DeathSequence::update(float dt) {
    if (m_phase == DeathPhase::None) return;

    m_timer += dt;

    if (m_phase == DeathPhase::Collapse) {
        if (m_timer >= 1.5f) {
            m_phase = DeathPhase::FadeToBlack;
            m_timer = 0.0f;
        }
    } else if (m_phase == DeathPhase::FadeToBlack) {
        if (m_timer >= 2.0f) {
            m_phase = DeathPhase::WakeUpFadeIn;
            m_timer = 0.0f;
        }
    } else if (m_phase == DeathPhase::WakeUpFadeIn) {
        if (m_timer >= 2.0f) {
            m_phase = DeathPhase::None;
            m_timer = 0.0f;
        }
    }
}

void DeathSequence::reset() {
    m_phase = DeathPhase::None;
    m_timer = 0.0f;
}

float DeathSequence::blackOverlayAlpha() const {
    if (m_phase == DeathPhase::Collapse) {
        return (m_timer / 1.5f) * 0.4f; // subtle darkening
    } else if (m_phase == DeathPhase::FadeToBlack) {
        return std::min(1.0f, 0.4f + (m_timer / 0.8f) * 0.6f);
    } else if (m_phase == DeathPhase::WakeUpFadeIn) {
        return std::max(0.0f, 1.0f - (m_timer / 2.0f));
    }
    return 0.0f;
}

float DeathSequence::cameraPitchDrop() const {
    if (m_phase == DeathPhase::Collapse) {
        // Dramatic camera roll/drop hitting floor
        return -65.0f * (m_timer / 1.5f);
    }
    return 0.0f;
}

void DeathSequence::pickWakeWhisper() {
    static const QString whispers[] = {
        "Mae: 'Breathe, Nora. The fire hasn't reached you yet...'",
        "Dr. Crane: 'Observation #14: The subject's subconscious rejects termination.'",
        "Nora: 'I... I'm still in the safe room. Was it just in my head?'",
        "Eli: 'Hide again, Nora... he's coming down the stairs...'",
        "Ada: 'Don't drop the light. They feed on what you forget.'"
    };
    int idx = (m_deathCount - 1) % 5;
    m_wakeWhisper = whispers[idx];
}

} // namespace hm
