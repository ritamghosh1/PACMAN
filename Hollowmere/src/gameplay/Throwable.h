#pragma once

#include "core/mathx.h"
#include "world/Collision.h"
#include "Item.h"

namespace hm {

class ThrowableEntity {
public:
    ThrowableEntity(ItemType type, const Vec3& spawnPos, const Vec3& launchDir, float speed = 9.5f);

    void update(float dt, const Collision& collision);

    bool isDead() const { return m_isDead; }
    const Vec3& position() const { return m_position; }
    ItemType type() const { return m_type; }

    bool didImpact() const { return m_didImpact; }
    void clearImpactFlag() { m_didImpact = false; }

private:
    ItemType m_type;
    Vec3 m_position;
    Vec3 m_velocity;
    bool m_isDead = false;
    bool m_didImpact = false;
    float m_lifetime = 0.0f;
};

} // namespace hm
