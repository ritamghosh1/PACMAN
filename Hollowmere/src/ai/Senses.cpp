#include "Senses.h"
#include <cmath>

namespace hm {

NoiseBus& NoiseBus::instance() {
    static NoiseBus s_bus;
    return s_bus;
}

void NoiseBus::emitNoise(const Vec3& pos, float radius, float loudness) {
    m_events.push_back({pos, radius, loudness, 0.5f});
}

void NoiseBus::update(float dt) {
    for (auto it = m_events.begin(); it != m_events.end();) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.0f) {
            it = m_events.erase(it);
        } else {
            ++it;
        }
    }
}

bool NoiseBus::hearsNoise(const Vec3& listenerPos, float hearingRadius, Vec3& outNoisePos) const {
    for (const auto& ev : m_events) {
        float d = distance(listenerPos, ev.position);
        if (d <= (hearingRadius + ev.radius)) {
            outNoisePos = ev.position;
            return true;
        }
    }
    return false;
}

bool Senses::canSee(const Vec3& eyePos,
                    const Vec3& eyeForward,
                    const Vec3& targetPos,
                    float fovDegrees,
                    float maxDistance,
                    const Collision& collision) {
    Vec3 toTarget = targetPos - eyePos;
    float dist = length(toTarget);
    if (dist < 0.001f) return true;
    if (dist > maxDistance) return false;

    Vec3 dir = toTarget * (1.0f / dist);
    Vec3 fwd = normalize(eyeForward);

    float cosAngle = dot(fwd, dir);
    float halfFovRad = radians(fovDegrees * 0.5f);
    if (cosAngle < std::cos(halfFovRad)) {
        return false; // Outside FOV cone
    }

    // Raycast vs collision colliders
    Ray ray;
    ray.origin = eyePos;
    ray.dir = dir;

    int hitTag, hitId;
    float hitDist;
    if (collision.raycast(ray, dist, hitTag, hitId, hitDist)) {
        if (hitDist < dist - 0.2f) {
            return false; // Obstructed by wall
        }
    }

    return true;
}

} // namespace hm
