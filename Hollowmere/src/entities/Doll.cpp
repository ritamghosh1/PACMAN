#include "Doll.h"
#include "ai/Pathfind.h"
#include <cmath>

namespace hm {

Doll::Doll(const Vec3& spawnPos)
    : Ghost(GhostType::Doll, spawnPos) {
    m_speed = 2.6f;
}

void Doll::update(float dt,
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
    if (m_teleportCooldown > 0.0f) {
        m_teleportCooldown -= dt;
    }

    if (m_state == GhostState::Stunned) {
        m_stunTimer -= dt;
        m_dissolve = 0.3f;
        if (m_stunTimer <= 0.0f) {
            m_state = GhostState::Investigate;
            m_dissolve = 0.0f;
        }
        return;
    }

    // Mercy counterplay: Peek-a-boo
    // If player closes eyes while Eli is within 5m, then opens them
    if (playerEyesClosed && m_distToPlayer < 6.0f) {
        m_eyesClosedTimer += dt;
    } else {
        if (m_eyesClosedTimer > 1.2f && !m_playedPeekABoo) {
            // Peek-a-boo success!
            m_playedPeekABoo = true;
            pacify();
            m_dissolve = 0.6f;
            return;
        }
        m_eyesClosedTimer = 0.0f;
    }
    if (m_state == GhostState::Pacified) return;

    // Outsmart counterplay: Direct lantern beam stuns Eli for 3 seconds
    if (playerLanternOn) {
        Vec3 toGhost = m_position - playerPos;
        float d = length(toGhost);
        if (d > 0.01f && d < 10.0f) {
            Vec3 dir = toGhost * (1.0f / d);
            if (dot(dir, normalize(playerForward)) > 0.80f) {
                Ray ray;
                ray.origin = playerPos + Vec3(0, 1.2f, 0);
                ray.dir = dir;
                int hitTag, hitId;
                float hitDist;
                if (!collision.raycast(ray, d, hitTag, hitId, hitDist) || hitDist >= d - 0.2f) {
                    stun(3.0f);
                    return;
                }
            }
        }
    }

    // Teleport ambush mechanic: If lantern is OFF and teleport cooldown ready,
    // teleport to point behind player (>4.5m away)
    if (!playerLanternOn && m_teleportCooldown <= 0.0f && m_distToPlayer > 8.0f) {
        Vec3 behindPlayer = playerPos - normalize(playerForward) * 5.0f;
        // Verify position doesn't collide with wall
        AABB testBox;
        testBox.min = behindPlayer - Vec3(0.3f, 0, 0.3f);
        testBox.max = behindPlayer + Vec3(0.3f, 1.5f, 0.3f);
        bool blocked = false;
        for (const auto& col : collision.colliders()) {
            if (col.enabled && col.box.intersects(testBox)) {
                blocked = true;
                break;
            }
        }
        if (!blocked) {
            m_position = behindPlayer;
            m_teleportCooldown = 25.0f;
            m_state = GhostState::Hunting;
        }
    }

    // Approach player in darkness
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
        m_position = collision.resolveMotion(m_position, targetMotion, 0.25f, 1.0f);
    }

    // Strike if close (≥1.5 s telegraph)
    if (m_distToPlayer < 1.3f && m_attackCooldown <= 0.0f) {
        m_telegraphTimer += dt;
        m_agitation = 0.9f;
        if (m_telegraphTimer > 1.5f) {
            m_hasAttacked = true;
            m_attackCooldown = 3.5f;
            m_telegraphTimer = 0.0f;
        }
    } else {
        m_telegraphTimer = 0.0f;
    }
}

} // namespace hm
