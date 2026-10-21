#pragma once

#include "core/mathx.h"
#include <QString>

namespace hm {

enum class DeathPhase {
    None,
    Collapse,       // 0 - 1.5s: First-person pitch down, ghost screamer close-up
    FadeToBlack,    // 1.5 - 3.5s: Blood on lens, heartbeat drop, darkness
    WakeUpFadeIn    // 3.5 - 5.5s: Nora gasping, waking in nearest safe room
};

class DeathSequence {
public:
    DeathSequence();

    void trigger(const Vec3& ghostPos);
    void update(float dt);
    void reset();

    bool isActive() const { return m_phase != DeathPhase::None; }
    DeathPhase phase() const { return m_phase; }
    float phaseTimer() const { return m_timer; }
    float blackOverlayAlpha() const;
    float cameraPitchDrop() const;
    QString wakeWhisper() const { return m_wakeWhisper; }

    int deathCount() const { return m_deathCount; }

private:
    DeathPhase m_phase = DeathPhase::None;
    float m_timer = 0.0f;
    Vec3 m_killerPos;
    int m_deathCount = 0;
    QString m_wakeWhisper;

    void pickWakeWhisper();
};

} // namespace hm
