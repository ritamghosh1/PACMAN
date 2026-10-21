#pragma once

#include "core/mathx.h"
#include <vector>

namespace hm {

struct PhantomGhost {
    Vec3 position;
    float timer = 0.0f;
    float maxLifetime = 3.0f;
    float alpha = 1.0f;
};

class HallucinationSystem {
public:
    HallucinationSystem();

    void update(float dt, float sanity, const Vec3& playerPos, float playerYaw);
    const std::vector<PhantomGhost>& phantomGhosts() const { return m_phantoms; }

    bool shouldTriggerPhantomStep() const { return m_phantomStepTriggered; }
    void clearPhantomStepTrigger() { m_phantomStepTriggered = false; }

    float peripheralShadowIntensity() const { return m_shadowIntensity; }

private:
    std::vector<PhantomGhost> m_phantoms;
    float m_spawnCooldown = 0.0f;
    float m_stepTimer = 0.0f;
    bool m_phantomStepTriggered = false;
    float m_shadowIntensity = 0.0f;
};

} // namespace hm
