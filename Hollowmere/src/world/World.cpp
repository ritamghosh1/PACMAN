#include "World.h"
#include <cmath>

namespace hm {

World::World() = default;

World::~World() = default;

bool World::init(QOpenGLFunctions_3_3_Core* gl) {
    if (!MapGen::generateManor(gl, m_mapData)) {
        return false;
    }

    m_player.spawn(m_mapData.playerSpawnPos);

    // Spawn Ghost in the sanitarium / library wing
    Vec3 ghostSpawn = m_mapData.playerSpawnPos + Vec3(12.0f, 0.0f, 8.0f);
    m_ghosts.emplace_back(ghostSpawn);
    for (auto& g : m_ghosts) {
        g.initMesh(gl);
    }

    m_audio.init();
    return true;
}

void World::update(float dt) {
    // 1. Update Doors
    for (auto& door : m_mapData.doors) {
        door.update(dt);
    }

    // 2. Update Pickups
    for (auto& pickup : m_mapData.pickups) {
        pickup.update(dt);
    }

    // 3. Update Player
    m_player.update(dt, m_mapData.collision, m_mapData.doors, m_mapData.pickups);

    // Footstep audio trigger
    if (m_player.checkFootstepTriggered()) {
        m_audio.playFootstep();
    }

    // 4. Update Ghosts
    float minGhostDist = 999.0f;
    for (auto& ghost : m_ghosts) {
        ghost.update(dt, m_player.position(),
                     m_player.lantern().isOn(),
                     m_player.lantern().isFocused(),
                     m_player.isSprinting(),
                     m_mapData.collision);

        float d = ghost.proximityDistance();
        if (d < minGhostDist) minGhostDist = d;

        // Ghost attack trigger
        if (ghost.hasAttacked()) {
            ghost.resetAttackFlag();
            m_player.takeDamage();
            m_audio.playJumpscareStinger();
            m_fearLevel = 1.0f;
        }

        // Positional weeping audio
        Vec3 toGhost = ghost.position() - m_player.position();
        float angle = std::atan2(toGhost.x, toGhost.z) - radians(m_player.yaw());
        m_audio.setGhostWailDistance(d, angle);
    }

    // 5. Update Fear level based on proximity and darkness
    float targetFear = 0.0f;
    if (minGhostDist < 12.0f) {
        targetFear = clamp(1.0f - (minGhostDist / 12.0f), 0.0f, 1.0f);
    }
    if (!m_player.lantern().isOn()) {
        targetFear = std::max(targetFear, 0.35f);
    }

    m_fearLevel += (targetFear - m_fearLevel) * std::min(1.0f, dt * 2.0f);

    // 6. Update Audio Engine listener
    m_audio.update(m_player.eyePosition(), m_player.yaw());
}

} // namespace hm
