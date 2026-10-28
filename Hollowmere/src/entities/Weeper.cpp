#include "Weeper.h"
#include "ai/Senses.h"
#include "ai/Pathfind.h"
#include <cmath>
#include <algorithm>

namespace hm {

Weeper::Weeper(const Vec3& spawnPos)
    : Ghost(GhostType::Weeper, spawnPos) {
    m_speed = 1.4f;
    m_patrolPoints = {
        spawnPos,
        spawnPos + Vec3(4.0f, 0.0f, 0.0f),
        spawnPos + Vec3(4.0f, 0.0f, 4.0f),
        spawnPos + Vec3(0.0f, 0.0f, 4.0f)
    };
}

void Weeper::update(float dt,
                    const Vec3& playerPos,
                    const Vec3& playerForward,
                    bool playerLanternOn,
                    bool playerLanternFocused,
                    bool playerSprinting,
                    bool /*playerEyesClosed*/,
                    bool playerHoldingBreath,
                    bool playerHumming,
                    Collision& collision,
                    const NavGrid* nav) {
    m_distToPlayer = distance(m_position, playerPos);

    if (m_attackCooldown > 0.0f) {
        m_attackCooldown -= dt;
    }

    // 1. Mercy counterplay: Humming lullaby
    if (playerHumming && m_distToPlayer < 10.0f) {
        pacify();
        m_pacifiedByHum = true;
        m_agitation = 0.0f;
        m_dissolve = 0.5f; // Sits in gentle translucent peace
        return;
    }
    if (m_state == GhostState::Pacified) return;

    // 2. Outsmart counterplay: Focused lantern beam burns and drives off
    if (playerLanternOn && playerLanternFocused) {
        Vec3 toGhost = normalize(m_position - playerPos);
        float alignment = dot(toGhost, normalize(playerForward));
        if (alignment > 0.88f && m_distToPlayer < 12.0f) {
            m_lanternExposureTimer += dt;
            m_agitation = 0.9f;
            if (m_lanternExposureTimer > 1.8f) {
                repel(playerPos);
                m_lanternExposureTimer = 0.0f;
            }
        } else {
            m_lanternExposureTimer = std::max(0.0f, m_lanternExposureTimer - dt);
        }
    } else {
        m_lanternExposureTimer = std::max(0.0f, m_lanternExposureTimer - dt);
    }

    // Handle fleeing state
    if (m_state == GhostState::Fleeing) {
        m_fleeTimer -= dt;
        m_dissolve = clamp(1.0f - (m_fleeTimer / 18.0f), 0.0f, 1.0f);
        if (m_fleeTimer <= 0.0f) {
            m_state = GhostState::Patrol;
            m_dissolve = 0.0f;
            m_agitation = 0.0f;
        }
        return;
    }

    // 3. Avoid counterplay: Hearing and sight detection
    bool detected = false;

    // Sight check
    Vec3 ghostFwd(std::sin(radians(m_yaw)), 0.0f, std::cos(radians(m_yaw)));
    if (Senses::canSee(m_position + Vec3(0, 1.5f, 0), ghostFwd, playerPos + Vec3(0, 1.5f, 0), 90.0f, 8.0f, collision)) {
        detected = true;
    }

    // Hearing check
    float hearingRad = playerSprinting ? 14.0f : (playerHoldingBreath ? 1.5f : 4.5f);
    Vec3 noisePos;
    if (NoiseBus::instance().hearsNoise(m_position, hearingRad, noisePos)) {
        detected = true;
        m_targetPos = noisePos;
    } else if (m_distToPlayer < hearingRad) {
        detected = true;
        m_targetPos = playerPos;
    }

    // State transitions
    if (detected) {
        m_state = GhostState::Hunting;
        m_agitation = 0.6f;
        m_targetPos = playerPos;
    } else if (m_state == GhostState::Hunting) {
        m_state = GhostState::Investigate;
    }

    // Navigation and path following
    m_repathTimer += dt;
    if (m_repathTimer > 0.6f && nav) {
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

    // Movement execution
    if (m_state == GhostState::Hunting || m_state == GhostState::Investigate) {
        Vec3 diff = moveTarget - m_position;
        diff.y = 0.0f;
        float d = length(diff);
        if (d > 0.05f) {
            Vec3 dir = diff * (1.0f / d);
            m_yaw = degrees(std::atan2(dir.x, dir.z));
            Vec3 targetMotion = m_position + dir * (m_speed * dt);
            m_position = collision.resolveMotion(m_position, targetMotion, 0.35f, 1.8f);
        }
    } else {
        // Patrol
        m_patrolTimer += dt;
        if (m_patrolTimer > 6.0f) {
            m_patrolTimer = 0.0f;
            m_currentPatrolIdx = (m_currentPatrolIdx + 1) % m_patrolPoints.size();
            m_targetPos = m_patrolPoints[m_currentPatrolIdx];
        }
        Vec3 diff = m_targetPos - m_position;
        diff.y = 0.0f;
        float d = length(diff);
        if (d > 0.1f) {
            Vec3 dir = diff * (1.0f / d);
            m_yaw = degrees(std::atan2(dir.x, dir.z));
            Vec3 targetMotion = m_position + dir * (m_speed * 0.7f * dt);
            m_position = collision.resolveMotion(m_position, targetMotion, 0.35f, 1.8f);
        }
    }

    // Attack telegraph
    if (m_distToPlayer < 1.4f && m_state == GhostState::Hunting) {
        m_telegraphTimer += dt;
        m_agitation = 1.0f;
        if (m_telegraphTimer > 1.5f && m_attackCooldown <= 0.0f) {
            m_hasAttacked = true;
            m_attackCooldown = 3.0f;
            m_telegraphTimer = 0.0f;
        }
    } else {
        m_telegraphTimer = 0.0f;
    }
}

} // namespace hm
