#pragma once

#include <vector>
#include "core/mathx.h"

namespace hm {

struct Collider {
    AABB box;
    int tag = 0; // 0: wall/static, 1: door, 2: interactable
    int id = 0;
    bool enabled = true;
};

class Collision {
public:
    Collision() = default;

    void clear();
    void addBox(const AABB& box, int tag = 0, int id = 0);
    void setColliderEnabled(int id, bool enabled);

    // Resolves circle/cylinder player sliding against AABBs on XZ plane
    Vec3 resolveMotion(const Vec3& currentPos, const Vec3& targetPos, float radius, float height);

    // Raycast to find closest hit collider
    bool raycast(const Ray& ray, float maxDist, int& hitTagOut, int& hitIdOut, float& hitDistOut) const;

    const std::vector<Collider>& colliders() const { return m_colliders; }

    // While set, motion resolution skips colliders with this tag (ghosts drift through doors).
    void setIgnoreTag(int tag) { m_ignoreTag = tag; }

private:
    std::vector<Collider> m_colliders;
    int m_ignoreTag = -1;
};

} // namespace hm
