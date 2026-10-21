#include "Pickup.h"
#include <cmath>

namespace hm {

Pickup::Pickup(int id, PickupType type, const Vec3& pos, const QString& name, const QString& desc)
    : m_id(id), m_type(type), m_position(pos), m_name(name), m_description(desc) {
}

void Pickup::update(float dt) {
    if (m_collected) return;
    m_bobTime += dt * 2.5f;
    m_rotAngle += dt * 45.0f;
    if (m_rotAngle >= 360.0f) m_rotAngle -= 360.0f;
}

Vec3 Pickup::renderPosition() const {
    float bob = std::sin(m_bobTime) * 0.05f;
    return m_position + Vec3(0, bob, 0);
}

AABB Pickup::bounds() const {
    return AABB(m_position - Vec3(0.4f, 0.4f, 0.4f), m_position + Vec3(0.4f, 0.4f, 0.4f));
}

} // namespace hm
