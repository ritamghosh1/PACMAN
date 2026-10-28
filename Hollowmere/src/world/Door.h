#pragma once

#include "core/mathx.h"
#include <QString>

namespace hm {

// A hinged door filling one full cell-wide gap. `spansX` doors run along the X axis
// (they close a passage that runs north-south), otherwise along Z.
class Door {
public:
    Door(int id, const Vec3& pos, bool spansX, int keyId = 0, float width = 2.4f);

    void update(float dt);

    bool tryInteract(int playerKeyFlags, QString& messageOut, bool fromInside = false);
    // The wing side of the door: a locked door can always be unlatched from there.
    void setInnerPoint(const Vec3& p) { m_inner = p; }
    bool isInsideSide(const Vec3& p) const { return dot(p - m_position, m_inner - m_position) > 0.0f; }
    void forceOpen();

    int id() const { return m_id; }
    const Vec3& position() const { return m_position; }
    bool isOpen() const { return m_isOpen; }
    bool isLocked() const { return m_isLocked; }
    bool spansX() const { return m_spansX; }
    int keyId() const { return m_keyId; }
    float width() const { return m_width; }
    // 0 closed .. 1 fully open; drives the swing animation
    float openAmount() const { return m_currentAngle / 95.0f; }
    float openAngle() const { return m_currentAngle; }

    // Hinge position and the model rotation about Y for the current swing
    Vec3 hingePosition() const;
    float baseYawDeg() const { return m_spansX ? 0.0f : 90.0f; }

    AABB currentColliderBox() const;
    static QString keyName(int keyId);

private:
    int m_id;
    Vec3 m_position;
    bool m_spansX;
    bool m_isOpen = false;
    bool m_isLocked = false;
    int m_keyId = 0;
    float m_width;
    Vec3 m_inner;

    float m_currentAngle = 0.0f;
    float m_targetAngle = 0.0f;
};

} // namespace hm
