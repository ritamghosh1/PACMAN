#include "Warden.h"
#include "ai/Senses.h"
#include "ai/Pathfind.h"
#include <cmath>

namespace hm {

Warden::Warden(const Vec3& spawnPos)
    : Ghost(GhostType::Warden, spawnPos), m_lastKnownPlayerPos(spawnPos) {
    m_speed = 2.7f; // Relentless, but a sprinting player out-runs him
}

void Warden::update(float dt,
                    const Vec3& playerPos,
                    const Vec3& playerForward,
                    bool playerLanternOn,
                    bool playerLanternFocused,
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
        m_agitation = 0.5f;
        if (m_stunTimer <= 0.0f) {
            m_state = GhostState::Hunting;
            m_agitation = 1.0f;
        }
        return;
    }

    m_keyJingleTimer += dt;
    m_roarTimer += dt;

    // Outsmart counterplay: Direct focused lantern beam stuns Warden briefly (2.0s)
    if (playerLanternOn && playerLanternFocused) {
        Vec3 toWarden = m_position - playerPos;
        float d = length(toWarden);
        if (d > 0.01f && d < 8.0f) {
            Vec3 dir = toWarden * (1.0f / d);
            if (dot(dir, normalize(playerForward)) > 0.88f) {
                stun(2.0f);
                return;
            }
        }
    }

    // Hybrid Senses: Vision check
    Vec3 wardenFwd(std::sin(radians(m_yaw)), 0.0f, std::cos(radians(m_yaw)));
    bool seesPlayer = Senses::canSee(m_position + Vec3(0, 2.1f, 0), wardenFwd,
                                    playerPos + Vec3(0, 1.6f, 0), 110.0f, 16.0f, collision);

    // Hearing check
    float hearingRad = playerSprinting ? 22.0f : (playerHoldingBreath ? 2.5f : 8.0f);
    Vec3 noisePos;
    bool hearsNoise = NoiseBus::instance().hearsNoise(m_position, hearingRad, noisePos);

    if (seesPlayer) {
        m_lastKnownPlayerPos = playerPos;
        m_state = GhostState::Hunting;
        m_agitation = 1.0f;
    } else if (hearsNoise) {
        m_lastKnownPlayerPos = noisePos;
        m_state = GhostState::Hunting;
        m_agitation = 0.85f;
    }

    // Navigate to last known player position
    m_repathTimer += dt;
    if (m_repathTimer > 0.35f && nav) {
        m_repathTimer = 0.0f;
        m_currentPath = Pathfind::findPath(*nav, m_position, m_lastKnownPlayerPos, true);
        m_pathIndex = 0;
    }

    Vec3 moveTarget = m_lastKnownPlayerPos;
    if (!m_currentPath.empty() && m_pathIndex < m_currentPath.size()) {
        moveTarget = m_currentPath[m_pathIndex];
        if (distance(m_position, moveTarget) < 0.7f) {
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
        m_position = collision.resolveMotion(m_position, targetMotion, 0.45f, 2.3f);
    }

    // Strike if close (1.2s telegraph)
    if (m_distToPlayer < 1.6f && m_attackCooldown <= 0.0f) {
        m_telegraphTimer += dt;
        m_agitation = 1.0f;
        if (m_telegraphTimer > 1.2f) {
            m_hasAttacked = true;
            m_attackCooldown = 3.0f;
            m_telegraphTimer = 0.0f;
        }
    } else {
        m_telegraphTimer = 0.0f;
    }
}

} // namespace hm
