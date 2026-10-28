#include "Door.h"
#include <algorithm>
#include <cmath>

namespace hm {

Door::Door(int id, const Vec3& pos, bool spansX, int keyId, float width)
    : m_id(id), m_position(pos), m_spansX(spansX), m_isLocked(keyId != 0), m_keyId(keyId), m_width(width) {
}

QString Door::keyName(int keyId) {
    switch (keyId) {
    case 1: return "Green Key";
    case 2: return "Rust Key";
    case 3: return "Nursery Key";
    case 4: return "Chapel Key";
    default: return "key";
    }
}

void Door::update(float dt) {
    float diff = m_targetAngle - m_currentAngle;
    m_currentAngle += diff * std::min(1.0f, dt * 5.0f);
    if (std::fabs(m_targetAngle - m_currentAngle) < 0.2f) m_currentAngle = m_targetAngle;
}

bool Door::tryInteract(int playerKeyFlags, QString& messageOut) {
    if (m_isLocked) {
        if (m_keyId != 0 && (playerKeyFlags & (1 << m_keyId))) {
            m_isLocked = false;
            messageOut = QString("Unlocked with the %1.").arg(keyName(m_keyId));
        } else {
            messageOut = QString("Locked. You need the %1.").arg(keyName(m_keyId));
            return false;
        }
    } else {
        messageOut = m_isOpen ? "Door closed." : "Door opened.";
    }
    m_isOpen = !m_isOpen;
    m_targetAngle = m_isOpen ? 95.0f : 0.0f;
    return true;
}

void Door::forceOpen() {
    m_isLocked = false;
    m_isOpen = true;
    m_targetAngle = 95.0f;
    m_currentAngle = 95.0f;
}

Vec3 Door::hingePosition() const {
    float h = m_width * 0.5f;
    return m_spansX ? m_position + Vec3(-h, 0, 0) : m_position + Vec3(0, 0, -h);
}

AABB Door::currentColliderBox() const {
    float h = m_width * 0.5f;
    if (m_spansX) return AABB(m_position - Vec3(h, 0.0f, 0.12f), m_position + Vec3(h, 2.6f, 0.12f));
    return AABB(m_position - Vec3(0.12f, 0.0f, h), m_position + Vec3(0.12f, 2.6f, h));
}

} // namespace hm
