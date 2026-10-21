#pragma once

#include "Ghost.h"

namespace hm {

class Warden : public Ghost {
public:
    Warden(const Vec3& spawnPos);

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

    bool brokeDoor() const { return m_brokeDoor; }
    void clearBrokeDoorFlag() { m_brokeDoor = false; }

private:
    Vec3 m_lastKnownPlayerPos;
    bool m_brokeDoor = false;
    float m_keyJingleTimer = 0.0f;
    float m_roarTimer = 0.0f;
};

} // namespace hm
