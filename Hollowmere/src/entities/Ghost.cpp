#include "Ghost.h"
#include "gfx/MeshBuilder.h"
#include <algorithm>
#include <cmath>

namespace hm {

Ghost::Ghost(const Vec3& spawnPos)
    : m_position(spawnPos), m_targetPos(spawnPos) {
    // Generate patrol waypoints around spawn
    m_patrolWaypoints.push_back(spawnPos);
    m_patrolWaypoints.push_back(spawnPos + Vec3(7.0f, 0.0f, 0.0f));
    m_patrolWaypoints.push_back(spawnPos + Vec3(7.0f, 0.0f, 8.0f));
    m_patrolWaypoints.push_back(spawnPos + Vec3(0.0f, 0.0f, 8.0f));
}

void Ghost::initMesh(QOpenGLFunctions_3_3_Core* gl) {
    MeshBuilder mb;

    // Translucent shrouded body
    Vec4 shroudColor(0.85f, 0.9f, 1.0f, 0.75f);
    Vec4 darkCloth(0.3f, 0.35f, 0.45f, 0.85f);
    Vec4 eyeGlow(1.0f, 0.2f, 0.2f, 1.0f);

    // Body / dress
    mb.addCylinder(Vec3(0, 0, 0), 0.45f, 1.4f, 12, darkCloth);

    // Torso / robe
    mb.addBox(Vec3(0, 1.3f, 0), Vec3(0.55f, 0.7f, 0.35f), Vec2(1, 1), shroudColor);

    // Head / cowl
    mb.addSphere(Vec3(0, 1.85f, 0), 0.28f, 10, 12, darkCloth);

    // Glowing red eyes
    mb.addSphere(Vec3(-0.09f, 1.88f, 0.22f), 0.045f, 6, 8, eyeGlow);
    mb.addSphere(Vec3( 0.09f, 1.88f, 0.22f), 0.045f, 6, 8, eyeGlow);

    // Long drooping clawed arms
    mb.addCylinder(Vec3(-0.35f, 0.5f, 0.0f), 0.08f, 0.9f, 8, shroudColor);
    mb.addCylinder(Vec3( 0.35f, 0.5f, 0.0f), 0.08f, 0.9f, 8, shroudColor);

    mb.build(gl, m_mesh);
}

void Ghost::update(float dt, const Vec3& playerPos, bool playerLanternOn, bool playerLanternFocused,
                   bool playerSprinting, Collision& collision) {
    m_distToPlayer = distance(m_position, playerPos);

    if (m_attackCooldown > 0.0f) {
        m_attackCooldown -= dt;
    }

    // 1. Check Flee condition: focused lantern beam directly illuminates ghost
    if (playerLanternOn && playerLanternFocused && m_distToPlayer < 9.0f) {
        m_state = GhostState::Flee;
        m_fleeTimer = 3.5f;
    }

    if (m_fleeTimer > 0.0f) {
        m_fleeTimer -= dt;
        m_state = GhostState::Flee;
        // Run away from player
        Vec3 awayDir = normalize(m_position - playerPos);
        m_targetPos = m_position + awayDir * 5.0f;
        m_speed = 3.0f;
    } else {
        // 2. Detection logic
        bool canHear = (playerSprinting && m_distToPlayer < 14.0f) || (m_distToPlayer < 4.0f);
        bool canSee = (m_distToPlayer < 10.0f && playerLanternOn);

        if (canSee || (canHear && m_distToPlayer < 7.0f)) {
            m_state = GhostState::Chase;
            m_targetPos = playerPos;
            m_speed = 2.4f;
        } else if (canHear) {
            m_state = GhostState::Investigate;
            m_targetPos = playerPos;
            m_speed = 1.7f;
        } else {
            // Patrol waypoints
            m_state = GhostState::Patrol;
            m_speed = 1.2f;

            m_patrolTimer += dt;
            if (distance(m_position, m_patrolWaypoints[m_currentWaypoint]) < 1.0f || m_patrolTimer > 10.0f) {
                m_patrolTimer = 0.0f;
                m_currentWaypoint = (m_currentWaypoint + 1) % m_patrolWaypoints.size();
            }
            m_targetPos = m_patrolWaypoints[m_currentWaypoint];
        }
    }

    // Move towards target position
    Vec3 moveDir = m_targetPos - m_position;
    moveDir.y = 0.0f;
    float distToTarget = length(moveDir);

    if (distToTarget > 0.2f) {
        moveDir = normalize(moveDir);
        Vec3 nextPos = m_position + moveDir * (m_speed * dt);
        m_position = collision.resolveMotion(m_position, nextPos, 0.4f, 2.0f);

        // Face movement direction
        m_yaw = degrees(std::atan2(moveDir.x, moveDir.z));
    }

    // Attack check
    if (m_distToPlayer < 1.3f && m_state == GhostState::Chase && m_attackCooldown <= 0.0f) {
        m_hasAttacked = true;
        m_attackCooldown = 2.5f;
    }
}

Mat4 Ghost::modelMatrix() const {
    Mat4 T = Mat4::translate(m_position);
    Mat4 R = Mat4::rotateY(radians(m_yaw));
    return T * R;
}

} // namespace hm
