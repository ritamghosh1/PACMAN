#include "Choir.h"
#include "ai/Senses.h"
#include "ai/Pathfind.h"
#include <cmath>

namespace hm {

Choir::Choir(const Vec3& spawnPos)
    : Ghost(GhostType::Choir, spawnPos) {
    m_speed = 1.6f;
    m_patrolRoute = {
        spawnPos,
        spawnPos + Vec3(6.0f, 0.0f, 0.0f),
        spawnPos + Vec3(6.0f, 0.0f, 8.0f),
        spawnPos + Vec3(-2.0f, 0.0f, 8.0f)
    };
}

void Choir::update(float dt,
                   const Vec3& playerPos,
                   const Vec3& /*playerForward*/,
                   bool /*playerLanternOn*/,
                   bool /*playerLanternFocused*/,
                   bool playerSprinting,
                   bool /*playerEyesClosed*/,
                   bool playerHoldingBreath,
                   bool /*playerHumming*/,
                   Collision& collision,
                   const NavGrid* nav) {
    m_distToPlayer = distance(m_position, playerPos);

    if (m_attackCooldown > 0.0f) {
        m_attackCooldown -= dt;
    }

    if (m_state == GhostState::Stunned) {
        m_stunTimer -= dt;
        if (m_stunTimer <= 0.0f) {
            m_state = GhostState::Patrol;
            m_agitation = 0.3f;
        }
        return;
    }

    // Mercy counterplay: If player stands completely still near Choir & holds breath
    if (playerHoldingBreath && !playerSprinting && m_distToPlayer < 6.0f) {
        m_standStillTimer += dt;
        if (m_standStillTimer > 3.0f) {
            m_singingPeacefully = true;
            m_agitation = 0.0f; // Pure harmony
            // Choir parts peacefully
            m_state = GhostState::Patrol;
        }
    } else {
        m_standStillTimer = 0.0f;
    }

    // Completely blind, relies 100% on hearing
    float hearingRad = playerSprinting ? 16.0f : (playerHoldingBreath ? 1.0f : 5.0f);
    Vec3 noisePos;
    bool heardNoise = false;

    if (NoiseBus::instance().hearsNoise(m_position, hearingRad, noisePos)) {
        heardNoise = true;
        m_targetPos = noisePos;
        m_state = GhostState::Hunting;
        m_agitation = 0.9f;
    } else if (m_distToPlayer < hearingRad && !m_singingPeacefully) {
        heardNoise = true;
        m_targetPos = playerPos;
        m_state = GhostState::Hunting;
        m_agitation = 0.9f;
    }

    if (!heardNoise && m_state == GhostState::Hunting) {
        m_state = GhostState::Investigate;
    }

    // Pathfinding
    m_repathTimer += dt;
    if (m_repathTimer > 0.5f && nav) {
        m_repathTimer = 0.0f;
        m_currentPath = Pathfind::findPath(*nav, m_position, m_targetPos, true);
        m_pathIndex = 0;
    }

    Vec3 moveTarget = m_targetPos;
    if (!m_currentPath.empty() && m_pathIndex < m_currentPath.size()) {
        moveTarget = m_currentPath[m_pathIndex];
        if (distance(m_position, moveTarget) < 0.6f) {
            m_pathIndex++;
        }
    }

    if (m_state == GhostState::Hunting || m_state == GhostState::Investigate) {
        Vec3 diff = moveTarget - m_position;
        diff.y = 0.0f;
        float d = length(diff);
        if (d > 0.05f) {
            Vec3 dir = diff * (1.0f / d);
            m_yaw = degrees(std::atan2(dir.x, dir.z));
            Vec3 targetMotion = m_position + dir * (m_speed * dt);
            m_position = collision.resolveMotion(m_position, targetMotion, 0.40f, 1.8f);
        }
    } else {
        // Patrol route
        Vec3 patrolTarget = m_patrolRoute[m_patrolIdx];
        Vec3 diff = patrolTarget - m_position;
        diff.y = 0.0f;
        float d = length(diff);
        if (d < 0.8f) {
            m_patrolIdx = (m_patrolIdx + 1) % m_patrolRoute.size();
        } else {
            Vec3 dir = diff * (1.0f / d);
            m_yaw = degrees(std::atan2(dir.x, dir.z));
            Vec3 targetMotion = m_position + dir * (m_speed * 0.75f * dt);
            m_position = collision.resolveMotion(m_position, targetMotion, 0.40f, 1.8f);
        }
    }

    // Strike if close and agitated
    if (m_distToPlayer < 1.4f && m_state == GhostState::Hunting && !m_singingPeacefully) {
        m_telegraphTimer += dt;
        if (m_telegraphTimer > 1.4f && m_attackCooldown <= 0.0f) {
            m_hasAttacked = true;
            m_attackCooldown = 3.5f;
            m_telegraphTimer = 0.0f;
        }
    } else {
        m_telegraphTimer = 0.0f;
    }
}

} // namespace hm
