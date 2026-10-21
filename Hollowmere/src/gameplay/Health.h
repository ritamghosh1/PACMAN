#pragma once

#include "core/mathx.h"

namespace hm {

enum class PlayerHealthState {
    Healthy,
    Wounded,
    Critical,
    Dead
};

class HealthSystem {
public:
    HealthSystem();

    void update(float dt, bool isMoving);

    void takeDamage();
    void applyBandage();
    void takePainkiller();

    PlayerHealthState state() const { return m_state; }
    bool isBleeding() const { return m_state == PlayerHealthState::Wounded || m_state == PlayerHealthState::Critical; }
    bool hasLimp() const { return (m_state == PlayerHealthState::Critical) && (m_painkillerTimer <= 0.0f); }

    bool isBandaging() const { return m_bandaging; }
    float bandageProgress() const { return m_bandageTimer / 6.0f; }
    void interruptBandaging() { m_bandaging = false; m_bandageTimer = 0.0f; }

    float speedMultiplier() const;
    float painkillerRemaining() const { return m_painkillerTimer; }

private:
    PlayerHealthState m_state = PlayerHealthState::Healthy;
    bool m_bandaging = false;
    float m_bandageTimer = 0.0f;
    float m_painkillerTimer = 0.0f;
};

} // namespace hm
