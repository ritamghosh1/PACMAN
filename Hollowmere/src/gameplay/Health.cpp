#include "Health.h"
#include <algorithm>

namespace hm {

HealthSystem::HealthSystem() = default;

void HealthSystem::update(float dt, bool isMoving) {
    if (m_painkillerTimer > 0.0f) {
        m_painkillerTimer -= dt;
        if (m_painkillerTimer < 0.0f) m_painkillerTimer = 0.0f;
    }

    if (m_bandaging) {
        if (isMoving) {
            interruptBandaging();
        } else {
            m_bandageTimer += dt;
            if (m_bandageTimer >= 6.0f) {
                m_bandaging = false;
                m_bandageTimer = 0.0f;
                // Heal one state step
                if (m_state == PlayerHealthState::Critical) {
                    m_state = PlayerHealthState::Wounded;
                } else if (m_state == PlayerHealthState::Wounded) {
                    m_state = PlayerHealthState::Healthy;
                }
            }
        }
    }
}

void HealthSystem::takeDamage() {
    interruptBandaging();
    switch (m_state) {
        case PlayerHealthState::Healthy:
            m_state = PlayerHealthState::Wounded;
            break;
        case PlayerHealthState::Wounded:
            m_state = PlayerHealthState::Critical;
            break;
        case PlayerHealthState::Critical:
            m_state = PlayerHealthState::Dead;
            break;
        case PlayerHealthState::Dead:
            break;
    }
}

void HealthSystem::applyBandage() {
    if (m_state != PlayerHealthState::Healthy && !m_bandaging) {
        m_bandaging = true;
        m_bandageTimer = 0.0f;
    }
}

void HealthSystem::takePainkiller() {
    m_painkillerTimer = 90.0f; // 90 seconds of no limp
}

float HealthSystem::speedMultiplier() const {
    if (m_state == PlayerHealthState::Critical && m_painkillerTimer <= 0.0f) {
        return 0.70f; // 30% speed reduction from limp
    }
    return 1.0f;
}

} // namespace hm
