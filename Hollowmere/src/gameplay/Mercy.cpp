#include "Mercy.h"
#include <cmath>

namespace hm {

MercySystem::MercySystem() = default;

void MercySystem::update(float dt, bool humHeld, bool adaNear) {
    bool wasActive = m_active;
    m_active = humHeld && adaNear && !m_adaPacified;
    if (!m_active) {
        if (wasActive || humHeld == false) m_humSuccessCount = 0;   // lose the thread when you stop humming
        return;
    }
    if (!wasActive) m_needle = 0.0f;
    m_needle += m_needleSpeed * dt;
    if (m_needle > 1.0f) {
        m_needle = 1.0f;
        m_needleSpeed = -std::abs(m_needleSpeed);
    } else if (m_needle < 0.0f) {
        m_needle = 0.0f;
        m_needleSpeed = std::abs(m_needleSpeed);
    }
}

void MercySystem::startHum() { m_active = true; }

int MercySystem::tap() {
    if (!m_active) return 0;
    if (std::abs(m_needle - 0.5f) < 0.12f) {
        if (++m_humSuccessCount >= requiredSuccesses()) {
            m_adaPacified = true;
            m_active = false;
        }
        return 1;
    }
    if (m_humSuccessCount > 0) --m_humSuccessCount;
    return 2;
}

bool MercySystem::processHumTap() { return tap() == 1; }

} // namespace hm
