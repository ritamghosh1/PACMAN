#pragma once

#include "core/mathx.h"
#include <QString>

namespace hm {

enum class PickupType {
    OilFlask,
    BrassKey,
    MemorySigil,
    Note,
    Bandage
};

class Pickup {
public:
    Pickup(int id, PickupType type, const Vec3& pos, const QString& name, const QString& desc = "");

    void update(float dt);

    int id() const { return m_id; }
    PickupType type() const { return m_type; }
    const Vec3& position() const { return m_position; }
    Vec3 renderPosition() const;
    float rotationAngle() const { return m_rotAngle; }

    const QString& name() const { return m_name; }
    const QString& description() const { return m_description; }

    bool isCollected() const { return m_collected; }
    void collect() { m_collected = true; }

    AABB bounds() const;

private:
    int m_id;
    PickupType m_type;
    Vec3 m_position;
    QString m_name;
    QString m_description;
    bool m_collected = false;

    float m_bobTime = 0.0f;
    float m_rotAngle = 0.0f;
};

} // namespace hm
