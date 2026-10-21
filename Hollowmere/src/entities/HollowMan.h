#pragma once

#include "Ghost.h"

namespace hm {

class HollowMan : public Ghost {
public:
    HollowMan(const Vec3& spawnPos);

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

    bool isFrozen() const { return m_isFrozen; }

private:
    bool m_isFrozen = false;
    float m_shearsTimer = 0.0f;
};

} // namespace hm
