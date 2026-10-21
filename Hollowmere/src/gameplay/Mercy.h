#pragma once

#include "core/mathx.h"

namespace hm {

class MercySystem {
public:
    MercySystem();

    void update(float dt);

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
    int requiredSuccesses() const { return 4; }

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
