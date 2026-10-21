#include "Mercy.h"
#include <cmath>

namespace hm {

MercySystem::MercySystem() = default;

void MercySystem::update(float dt) {
    if (m_humBeatTimer > 0.0f) {
        m_humBeatTimer -= dt;
    }
    if (m_active) {
        m_needle += m_needleSpeed * dt;
        if (m_needle > 1.0f) {
            m_needle = 1.0f;
            m_needleSpeed = -std::abs(m_needleSpeed);
        } else if (m_needle < 0.0f) {
            m_needle = 0.0f;
            m_needleSpeed = std::abs(m_needleSpeed);
        }
    }
}

void MercySystem::startHum() {
    m_active = true;
    m_humBeatTimer = 0.88f; // 68 BPM beat interval
}

bool MercySystem::processHumTap() {
    // Check if tap falls within generous rhythm window (±0.25s of beat)
    if (std::abs(m_humBeatTimer - 0.44f) < 0.28f) {
        m_humSuccessCount++;
        m_humBeatTimer = 0.88f;
        if (m_humSuccessCount >= 4) {
            m_adaPacified = true;
        }
        return true;
    }
    m_humBeatTimer = 0.88f;
    return false;
}

} // namespace hm
