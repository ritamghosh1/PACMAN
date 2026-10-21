#include "HollowMan.h"
#include "ai/Pathfind.h"
#include <cmath>

namespace hm {

HollowMan::HollowMan(const Vec3& spawnPos)
    : Ghost(GhostType::HollowMan, spawnPos) {
    m_speed = 3.0f; // Fast when unobserved
}

void HollowMan::update(float dt,
                       const Vec3& playerPos,
                       const Vec3& playerForward,
                       bool playerLanternOn,
                       bool /*playerLanternFocused*/,
                       bool /*playerSprinting*/,
                       bool playerEyesClosed,
                       bool /*playerHoldingBreath*/,
                       bool /*playerHumming*/,
                       Collision& collision,
                       const NavGrid* nav) {
    m_distToPlayer = distance(m_position, playerPos);

    if (m_attackCooldown > 0.0f) {
        m_attackCooldown -= dt;
    }

    m_shearsTimer += dt;

    // Check if player is directly observing Hollow Man:
    // 1. Lantern must be ON
    // 2. Eyes not closed
    // 3. Hollow Man inside player's forward view cone (dot > 0.65)
    // 4. Line of sight clear (no wall between player and Hollow Man)
    bool observed = false;
    if (playerLanternOn && !playerEyesClosed) {
        Vec3 toGhost = m_position - playerPos;
        float d = length(toGhost);
        if (d > 0.01f && d < 18.0f) {
            Vec3 dir = toGhost * (1.0f / d);
            float alignment = dot(dir, normalize(playerForward));
            if (alignment > 0.65f) { // ~80 degree cone
                Ray ray;
                ray.origin = playerPos + Vec3(0, 1.6f, 0);
                ray.dir = dir;
                int hitTag, hitId;
                float hitDist;
                if (!collision.raycast(ray, d, hitTag, hitId, hitDist) || hitDist >= d - 0.2f) {
                    observed = true;
                }
            }
        }
    }

    m_isFrozen = observed;

    if (m_isFrozen) {
        // Freeze solid in place!
        m_state = GhostState::Idle;
        m_agitation = 0.2f;
        m_telegraphTimer = 0.0f;
        return;
    }

    // Unobserved: stalks rapidly towards player
    m_state = GhostState::Hunting;
    m_agitation = 0.85f;

    m_repathTimer += dt;
    if (m_repathTimer > 0.4f && nav) {
        m_repathTimer = 0.0f;
        m_currentPath = Pathfind::findPath(*nav, m_position, playerPos, true);
        m_pathIndex = 0;
    }

    Vec3 moveTarget = playerPos;
    if (!m_currentPath.empty() && m_pathIndex < m_currentPath.size()) {
        moveTarget = m_currentPath[m_pathIndex];
        if (distance(m_position, moveTarget) < 0.6f) {
            m_pathIndex++;
        }
    }

    Vec3 diff = moveTarget - m_position;
    diff.y = 0.0f;
    float d = length(diff);
    if (d > 0.05f) {
        Vec3 dir = diff * (1.0f / d);
        m_yaw = degrees(std::atan2(dir.x, dir.z));
        Vec3 targetMotion = m_position + dir * (m_speed * dt);
        m_position = collision.resolveMotion(m_position, targetMotion, 0.35f, 2.1f);
    }

    // Strike if close
    if (m_distToPlayer < 1.4f && m_attackCooldown <= 0.0f) {
        m_telegraphTimer += dt;
        if (m_telegraphTimer > 1.2f) {
            m_hasAttacked = true;
            m_attackCooldown = 3.5f;
            m_telegraphTimer = 0.0f;
        }
    } else {
        m_telegraphTimer = 0.0f;
    }
}

} // namespace hm
