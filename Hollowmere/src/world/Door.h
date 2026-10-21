#pragma once

#include "core/mathx.h"
#include <QString>

namespace hm {

class Door {
public:
    Door(int id, const Vec3& pos, float angleDeg, bool locked = false, int keyId = 0);

    void update(float dt);

    bool tryInteract(int playerKeyFlags, QString& messageOut);
    void forceOpen();

    int id() const { return m_id; }
    const Vec3& position() const { return m_position; }
    bool isOpen() const { return m_isOpen; }
    bool isLocked() const { return m_isLocked; }
    float openAngle() const { return m_currentAngle; }

    AABB currentColliderBox() const;

private:
    int m_id;
    Vec3 m_position;
    bool m_isOpen = false;
    bool m_isLocked = false;
    int m_keyIdNeeded = 0;

    float m_currentAngle = 0.0f;
    float m_targetAngle = 0.0f;
};

} // namespace hm
