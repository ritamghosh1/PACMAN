#pragma once

#include "Ghost.h"

namespace hm {

class Doll : public Ghost {
public:
    Doll(const Vec3& spawnPos);

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

    bool playedPeekABoo() const { return m_playedPeekABoo; }

private:
    float m_teleportCooldown = 0.0f;
    float m_eyesClosedTimer = 0.0f;
    bool m_playedPeekABoo = false;
};

} // namespace hm
