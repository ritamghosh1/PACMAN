#pragma once

#include "core/mathx.h"

namespace hm {

class MercySystem {
public:
    MercySystem();

    // Humming only does something while Ada is within earshot; the panel is visible only then.
    void update(float dt, bool humHeld, bool adaNear);
    // Returns 1 on a good tap, 2 on a miss, 0 when the mini-game is not active.
    int tap();

    // Mini-game 1: Hum Lullaby (Ada)
    void startHum();
    bool processHumTap(); // Call when rhythm pulse key pressed
    bool isAdaPacified() const { return m_adaPacified; }

    // Mini-game 2: Peek-a-boo (Eli)
    void registerPeekABooSuccess() { m_eliPacified = true; }
    bool isEliPacified() const { return m_eliPacified; }

    // Mini-game 3: Stand Still Harmony (The Choir)
    void registerChoirHarmonySuccess() { m_choirPacified = true; }
    bool isChoirPacified() const { return m_choirPacified; }

    int totalGhostsPacified() const {
        return (m_adaPacified ? 1 : 0) + (m_eliPacified ? 1 : 0) + (m_choirPacified ? 1 : 0);
    }

    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }
    float needlePos() const { return m_needle; }
    int successCount() const { return m_humSuccessCount; }
    int requiredSuccesses() const { return 3; }

private:
    bool m_active = false;
    float m_needle = 0.5f;
    float m_needleSpeed = 1.2f;
    bool m_adaPacified = false;
    bool m_eliPacified = false;
    bool m_choirPacified = false;

    float m_humBeatTimer = 0.0f;
    int m_humSuccessCount = 0;
};

} // namespace hm
