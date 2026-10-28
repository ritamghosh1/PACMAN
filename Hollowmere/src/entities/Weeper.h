#pragma once

#include "Ghost.h"

namespace hm {

class Weeper : public Ghost {
public:
    Weeper(const Vec3& spawnPos);

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

    bool wasPacifiedByHum() const { return m_pacifiedByHum; }
    void setPatrolRoute(const std::vector<Vec3>& r) override { if (!r.empty()) { m_patrolPoints = r; m_currentPatrolIdx = 0; m_targetPos = r[0]; } }

private:
    float m_lanternExposureTimer = 0.0f;
    bool m_pacifiedByHum = false;
    float m_patrolTimer = 0.0f;
    std::vector<Vec3> m_patrolPoints;
    size_t m_currentPatrolIdx = 0;
};

} // namespace hm
