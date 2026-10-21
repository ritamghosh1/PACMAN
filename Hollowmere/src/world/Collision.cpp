#include "Collision.h"
#include <algorithm>

namespace hm {

void Collision::clear() {
    m_colliders.clear();
}

void Collision::addBox(const AABB& box, int tag, int id) {
    m_colliders.push_back({box, tag, id, true});
}

void Collision::setColliderEnabled(int id, bool enabled) {
    for (auto& c : m_colliders) {
        if (c.id == id) {
            c.enabled = enabled;
        }
    }
}

Vec3 Collision::resolveMotion(const Vec3& /*currentPos*/, const Vec3& targetPos, float radius, float height) {
    Vec3 pos = targetPos;

    // Helper: test & resolve circle vs 2D AABB on XZ plane
    auto resolveCircleAABB = [&](float& px, float& pz, float yPos, const AABB& box) {
        // Check Y overlap
        if (yPos + height < box.min.y || yPos > box.max.y) return;

        // Closest point on AABB to circle center
        float closestX = clamp(px, box.min.x, box.max.x);
        float closestZ = clamp(pz, box.min.z, box.max.z);

        float dx = px - closestX;
        float dz = pz - closestZ;
        float distSq = dx * dx + dz * dz;

        if (distSq < radius * radius && distSq > 1e-8f) {
            float dist = std::sqrt(distSq);
            float penetration = radius - dist;
            px += (dx / dist) * penetration;
            pz += (dz / dist) * penetration;
        } else if (distSq <= 1e-8f) {
            // Circle center is inside AABB: push out to nearest edge
            float toMinX = px - box.min.x;
            float toMaxX = box.max.x - px;
            float toMinZ = pz - box.min.z;
            float toMaxZ = box.max.z - pz;
            float minVal = std::min({toMinX, toMaxX, toMinZ, toMaxZ});

            if (minVal == toMinX) px = box.min.x - radius;
            else if (minVal == toMaxX) px = box.max.x + radius;
            else if (minVal == toMinZ) pz = box.min.z - radius;
            else pz = box.max.z + radius;
        }
    };

    // 2 iterations of relaxation for corner sliding
    for (int iter = 0; iter < 2; ++iter) {
        for (const auto& c : m_colliders) {
            if (!c.enabled) continue;
            resolveCircleAABB(pos.x, pos.z, pos.y, c.box);
        }
    }

    return pos;
}

bool Collision::raycast(const Ray& ray, float maxDist, int& hitTagOut, int& hitIdOut, float& hitDistOut) const {
    float closestDist = maxDist;
    bool hitAny = false;

    for (const auto& c : m_colliders) {
        if (!c.enabled) continue;
        float t = 0.0f;
        if (ray.intersectAABB(c.box, t)) {
            if (t > 0.0f && t < closestDist) {
                closestDist = t;
                hitTagOut = c.tag;
                hitIdOut = c.id;
                hitAny = true;
            }
        }
    }

    if (hitAny) {
        hitDistOut = closestDist;
        return true;
    }
    return false;
}

} // namespace hm
