#include "Hallucinations.h"
#include <cmath>
#include <algorithm>

namespace hm {

HallucinationSystem::HallucinationSystem() = default;

void HallucinationSystem::update(float dt, float sanity, const Vec3& playerPos, float playerYaw) {
    // Peripheral shadow intensity scales inversely with sanity (starts below 50% sanity)
    if (sanity < 50.0f) {
        m_shadowIntensity = (50.0f - sanity) / 50.0f;
    } else {
        m_shadowIntensity = 0.0f;
    }

    // Phantom footsteps in distance when sanity is low
    if (sanity < 35.0f) {
        m_stepTimer += dt;
        if (m_stepTimer > 4.5f) {
            m_stepTimer = 0.0f;
            m_phantomStepTriggered = true;
        }
    }

    // Phantom ghost apparitions spawn in peripheral view when sanity < 25%
    if (sanity < 25.0f) {
        m_spawnCooldown += dt;
        if (m_spawnCooldown > 12.0f && m_phantoms.size() < 2) {
            m_spawnCooldown = 0.0f;
            // Spawn just outside direct view (at ~70 degrees to player yaw)
            float angle = radians(playerYaw + 65.0f);
            float dist = 4.5f;
            Vec3 spawnPos(playerPos.x + std::sin(angle) * dist,
                          playerPos.y,
                          playerPos.z + std::cos(angle) * dist);
            m_phantoms.push_back({spawnPos, 0.0f, 3.5f, 0.0f});
        }
    }

    // Update active phantoms
    for (auto it = m_phantoms.begin(); it != m_phantoms.end(); ) {
        it->timer += dt;
        if (it->timer < 0.5f) {
            it->alpha = it->timer / 0.5f;
        } else if (it->timer > it->maxLifetime - 1.0f) {
            it->alpha = (it->maxLifetime - it->timer) / 1.0f;
        } else {
            it->alpha = 1.0f;
        }

        if (it->timer >= it->maxLifetime) {
            it = m_phantoms.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace hm
