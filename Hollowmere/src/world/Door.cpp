#include "Door.h"
#include <cmath>

namespace hm {

Door::Door(int id, const Vec3& pos, float /*angleDeg*/, bool locked, int keyId)
    : m_id(id), m_position(pos), m_isLocked(locked), m_keyIdNeeded(keyId) {
}

void Door::update(float dt) {
    float diff = m_targetAngle - m_currentAngle;
    m_currentAngle += diff * std::min(1.0f, dt * 6.0f);
}

bool Door::tryInteract(int playerKeyFlags, QString& messageOut) {
    if (m_isLocked) {
        if (m_keyIdNeeded != 0 && (playerKeyFlags & (1 << m_keyIdNeeded))) {
            m_isLocked = false;
            messageOut = "Unlocked with Brass Key.";
        } else {
            messageOut = "Locked. Needs a key.";
            return false;
        }
    }

    m_isOpen = !m_isOpen;
    m_targetAngle = m_isOpen ? 85.0f : 0.0f;
    messageOut = m_isOpen ? "Door opened." : "Door closed.";
    return true;
}

void Door::forceOpen() {
    m_isLocked = false;
    m_isOpen = true;
    m_targetAngle = 85.0f;
}

AABB Door::currentColliderBox() const {
    if (m_isOpen) {
        // When open, collider is rotated to wall side or slimmed
        return AABB(m_position - Vec3(0.1f, 0.0f, 0.1f), m_position + Vec3(0.1f, 2.4f, 0.1f));
    }
    // Closed door blocks hallway (e.g. 1.8m wide, 2.4m high)
    return AABB(m_position - Vec3(0.9f, 0.0f, 0.2f), m_position + Vec3(0.9f, 2.4f, 0.2f));
}

} // namespace hm
