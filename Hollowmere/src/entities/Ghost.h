#pragma once

#include "core/mathx.h"
#include "world/Collision.h"
#include "gfx/Mesh.h"
#include <vector>

namespace hm {

enum class GhostState {
    Patrol,
    Investigate,
    Chase,
    Flee
};

class Ghost {
public:
    Ghost(const Vec3& spawnPos);

    void initMesh(QOpenGLFunctions_3_3_Core* gl);
    void update(float dt, const Vec3& playerPos, bool playerLanternOn, bool playerLanternFocused,
                bool playerSprinting, Collision& collision);

    const Vec3& position() const { return m_position; }
    GhostState state() const { return m_state; }
    float proximityDistance() const { return m_distToPlayer; }
    bool hasAttacked() const { return m_hasAttacked; }
    void resetAttackFlag() { m_hasAttacked = false; }

    const Mesh& mesh() const { return m_mesh; }
    Mat4 modelMatrix() const;

private:
    Vec3 m_position;
    Vec3 m_targetPos;
    GhostState m_state = GhostState::Patrol;

    float m_speed = 1.4f;
    float m_yaw = 0.0f;
    float m_distToPlayer = 999.0f;

    float m_fleeTimer = 0.0f;
    float m_patrolTimer = 0.0f;
    bool m_hasAttacked = false;
    float m_attackCooldown = 0.0f;

    Mesh m_mesh;
    std::vector<Vec3> m_patrolWaypoints;
    int m_currentWaypoint = 0;
};

} // namespace hm
