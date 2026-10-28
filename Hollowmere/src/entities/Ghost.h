#pragma once

#include "core/mathx.h"
#include "world/Collision.h"
#include "world/NavGrid.h"
#include "gfx/GhostRenderer.h"
#include <vector>

namespace hm {

enum class GhostType {
    Weeper,
    HollowMan,
    Choir,
    Doll,
    Warden
};

enum class GhostState {
    Idle,
    Patrol,
    Investigate,
    Hunting,
    Telegraphing,
    Stunned,
    Fleeing,
    Pacified
};

class Ghost {
public:
    Ghost(GhostType type, const Vec3& spawnPos);
    virtual ~Ghost() = default;

    virtual void update(float dt,
                        const Vec3& playerPos,
                        const Vec3& playerForward,
                        bool playerLanternOn,
                        bool playerLanternFocused,
                        bool playerSprinting,
                        bool playerEyesClosed,
                        bool playerHoldingBreath,
                        bool playerHumming,
                        Collision& collision,
                        const NavGrid* nav = nullptr) = 0;

    GhostType type() const { return m_type; }
    // Called instead of update() while the player is far away / in another wing.
    void setDormant() { m_distToPlayer = 999.0f; m_telegraphTimer = 0.0f; m_agitation = 0.0f; }
    virtual void setPatrolRoute(const std::vector<Vec3>&) {}
    void resetToHome(const Vec3& home) {
        m_position = home; m_targetPos = home; m_state = GhostState::Patrol; m_currentPath.clear(); m_pathIndex = 0; m_dissolve = 0.0f;
    }
    GhostVisualType visualType() const { return m_visualType; }
    GhostState state() const { return m_state; }

    const Vec3& position() const { return m_position; }
    void setPosition(const Vec3& pos) { m_position = pos; }
    float yaw() const { return m_yaw; }
    float proximityDistance() const { return m_distToPlayer; }

    float dissolve() const { return m_dissolve; }
    float agitation() const { return m_agitation; }

    bool hasAttacked() const { return m_hasAttacked; }
    void resetAttackFlag() { m_hasAttacked = false; }

    virtual void stun(float durationSec);
    virtual void repel(const Vec3& sourcePos);
    virtual void pacify();

    Mat4 modelMatrix() const;

protected:
    GhostType m_type;
    GhostVisualType m_visualType;
    GhostState m_state = GhostState::Patrol;

    Vec3 m_position;
    Vec3 m_targetPos;
    float m_yaw = 0.0f;
    float m_speed = 1.4f;
    float m_distToPlayer = 999.0f;

    float m_dissolve = 0.0f;
    float m_agitation = 0.0f;

    float m_telegraphTimer = 0.0f;
    float m_stunTimer = 0.0f;
    float m_fleeTimer = 0.0f;
    bool m_hasAttacked = false;
    float m_attackCooldown = 0.0f;

    std::vector<Vec3> m_currentPath;
    size_t m_pathIndex = 0;
    float m_repathTimer = 0.0f;
};

} // namespace hm
