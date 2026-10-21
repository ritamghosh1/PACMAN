#pragma once

#include "core/mathx.h"
#include "gfx/Light.h"

namespace hm {

class Lantern {
public:
    Lantern();

    void update(float dt, const Vec3& playerPos, const Vec3& cameraForward, const Vec3& cameraRight, bool isMoving);

    void toggle();
    void setFocused(bool focused);
    bool isFocused() const { return m_focused; }
    bool isOn() const { return m_isOn; }

    void addOil(float amount);
    float oil() const { return m_oil; }

    SpotLight spotLight() const;
    const Vec3& heldPosition() const { return m_heldPos; }

private:
    bool m_isOn = true;
    bool m_focused = false;
    float m_oil = 100.0f; // 0 to 100%

    Vec3 m_heldPos;
    Vec3 m_lightDir;

    float m_flickerTime = 0.0f;
    float m_flickerIntensity = 1.0f;

    float m_bobTime = 0.0f;
};

} // namespace hm
