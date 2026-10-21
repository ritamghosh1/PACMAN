#include "Verbs.h"
#include <algorithm>

namespace hm {

AdvancedVerbs::AdvancedVerbs() = default;

void AdvancedVerbs::update(float dt,
                           bool keyAltDown,
                           bool keyCtrlDown,
                           bool keyShiftDown,
                           bool keyCDown) {
    m_isListening = keyAltDown;
    m_isEyesClosed = keyCtrlDown;

    bool requestHoldBreath = (keyShiftDown && keyCDown);
    if (requestHoldBreath && m_breathHoldTimer > 0.0f) {
        m_isHoldingBreath = true;
        m_breathHoldTimer -= dt;
        if (m_breathHoldTimer < 0.0f) {
            m_breathHoldTimer = 0.0f;
            m_isHoldingBreath = false;
        }
    } else {
        m_isHoldingBreath = false;
        // Regain breath hold capacity
        m_breathHoldTimer = std::min(8.0f, m_breathHoldTimer + dt * 2.0f);
    }
}

void AdvancedVerbs::enterHidingSpot(const Vec3& spotPos) {
    m_isHiding = true;
    m_hidingPos = spotPos;
}

void AdvancedVerbs::exitHidingSpot() {
    m_isHiding = false;
}

} // namespace hm
