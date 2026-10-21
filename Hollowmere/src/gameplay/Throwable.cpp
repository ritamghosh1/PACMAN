#include "Throwable.h"
#include "ai/Senses.h"

namespace hm {

ThrowableEntity::ThrowableEntity(ItemType type, const Vec3& spawnPos, const Vec3& launchDir, float speed)
    : m_type(type), m_position(spawnPos) {
    Vec3 dir = normalize(launchDir);
    // Slight upward arc
    dir.y += 0.22f;
    dir = normalize(dir);
    m_velocity = dir * speed;
}

void ThrowableEntity::update(float dt, const Collision& collision) {
    if (m_isDead) return;

    m_lifetime += dt;
    if (m_lifetime > 8.0f) {
        m_isDead = true;
        return;
    }

    // Apply gravity
    m_velocity.y -= 9.8f * dt;

    Vec3 nextPos = m_position + m_velocity * dt;

    // Check collision along flight trajectory
    Ray ray(m_position, normalize(m_velocity));
    float stepDist = length(m_velocity * dt);

    int hitTag, hitId;
    float hitDist;
    if (collision.raycast(ray, stepDist, hitTag, hitId, hitDist) || nextPos.y <= 0.05f) {
        // Impact!
        m_position = m_position + ray.dir * hitDist;
        m_didImpact = true;
        m_isDead = true;

        // Emit noise event on NoiseBus to distract ghosts
        float noiseRadius = (m_type == ItemType::Bottle) ? 12.0f : 8.0f;
        NoiseBus::instance().emitNoise(m_position, noiseRadius, 1.0f);
        return;
    }

    m_position = nextPos;
}

} // namespace hm
