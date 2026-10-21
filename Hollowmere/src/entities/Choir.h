#pragma once

#include "Ghost.h"

namespace hm {

class Choir : public Ghost {
public:
    Choir(const Vec3& spawnPos);

    void update(float dt,
                const Vec3& playerPos,
                const Vec3& playerForward,
                bool playerLanternOn,
                bool playerLanternFocused,
                bool playerSprinting,
                bool playerEyesClosed,
                bool playerHoldingBreath,
                bool playerHumming,
                Collision& collision,
                const NavGrid* nav = nullptr) override;

    bool isSingingPeacefully() const { return m_singingPeacefully; }

private:
    float m_standStillTimer = 0.0f;
    bool m_singingPeacefully = false;
    std::vector<Vec3> m_patrolRoute;
    size_t m_patrolIdx = 0;
};

} // namespace hm
