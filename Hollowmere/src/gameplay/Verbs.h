#pragma once

#include "core/mathx.h"

namespace hm {

class AdvancedVerbs {
public:
    AdvancedVerbs();

    void update(float dt,
                bool keyAltDown,
                bool keyCtrlDown,
                bool keyShiftDown,
                bool keyCDown);

    bool isListening() const { return m_isListening; }
    bool isEyesClosed() const { return m_isEyesClosed; }
    bool isHoldingBreath() const { return m_isHoldingBreath; }
    bool isHiding() const { return m_isHiding; }

    void enterHidingSpot(const Vec3& spotPos);
    void exitHidingSpot();

    float breathHoldRemaining() const { return m_breathHoldTimer; }

private:
    bool m_isListening = false;
    bool m_isEyesClosed = false;
    bool m_isHoldingBreath = false;
    bool m_isHiding = false;

    float m_breathHoldTimer = 8.0f; // Max 8s breath hold before gasping
    Vec3 m_hidingPos{0, 0, 0};
};

} // namespace hm
