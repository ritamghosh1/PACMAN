#pragma once

#include <vector>
#include "core/mathx.h"
#include "world/Collision.h"

namespace hm {

struct NoiseEvent {
    Vec3 position;
    float radius = 5.0f;
    float loudness = 1.0f;
    float lifetime = 0.5f;
};

class NoiseBus {
public:
    static NoiseBus& instance();

    void emitNoise(const Vec3& pos, float radius, float loudness = 1.0f);
    void update(float dt);
    const std::vector<NoiseEvent>& events() const { return m_events; }
    void clear() { m_events.clear(); }

    // Check if a listener at pos with hearingRadius hears any active noise
    bool hearsNoise(const Vec3& listenerPos, float hearingRadius, Vec3& outNoisePos) const;

private:
    NoiseBus() = default;
    std::vector<NoiseEvent> m_events;
};

class Senses {
public:
    // Check if eyePos can see targetPos within FOV cone, maxDist, and unobstructed by collision
    static bool canSee(const Vec3& eyePos,
                       const Vec3& eyeForward,
                       const Vec3& targetPos,
                       float fovDegrees,
                       float maxDistance,
                       const Collision& collision);
};

} // namespace hm
