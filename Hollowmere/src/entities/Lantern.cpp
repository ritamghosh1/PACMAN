#include "Lantern.h"
#include "core/config.h"
#include <algorithm>
#include <cmath>

namespace hm {

Lantern::Lantern() = default;

void Lantern::toggle() {
    if (m_oil > 0.0f) {
        m_isOn = !m_isOn;
    }
}

void Lantern::setFocused(bool focused) {
    m_focused = focused && m_isOn && m_oil > 0.0f;
}

void Lantern::addOil(float amount) {
    m_oil = std::min(config::tune::LANTERN_MAX_OIL, m_oil + amount);
    if (m_oil > 0.0f && !m_isOn) {
        m_isOn = true;
    }
}

void Lantern::update(float dt, const Vec3& playerPos, const Vec3& cameraForward, const Vec3& cameraRight, bool isMoving) {
    // Oil consumption
    if (m_isOn) {
        float drain = m_focused ? config::tune::LANTERN_OIL_DRAIN_FOCUSED : config::tune::LANTERN_OIL_DRAIN_NORMAL;
        m_oil = std::max(0.0f, m_oil - drain * dt);
        if (m_oil <= 0.0f) {
            m_isOn = false;
            m_focused = false;
        }
    }

    // Hand bob and sway
    if (isMoving) {
        m_bobTime += dt * 8.0f;
    } else {
        m_bobTime += dt * 1.5f;
    }

    float swayX = std::sin(m_bobTime) * (isMoving ? 0.04f : 0.015f);
    float swayY = std::cos(m_bobTime * 2.0f) * (isMoving ? 0.03f : 0.01f);

    // Position lantern slightly down, to the right, and forward from the eye
    m_heldPos = playerPos + (cameraRight * (0.28f + swayX)) + Vec3(0, -0.32f + swayY, 0) + (cameraForward * 0.45f);
    m_lightDir = cameraForward;

    // Organic flame flicker calculation
    m_flickerTime += dt * 12.0f;
    float microFlicker = (std::sin(m_flickerTime * 3.7f) * 0.04f + std::sin(m_flickerTime * 7.1f) * 0.02f);

    if (m_oil < 20.0f) {
        // Violently sputter and drop when oil is starving
        float dyingSputter = (std::sin(m_flickerTime * 15.0f) > 0.6f) ? -0.45f : 0.0f;
        m_flickerIntensity = 0.5f + microFlicker + dyingSputter;
    } else {
        m_flickerIntensity = 1.0f + microFlicker;
    }
}

SpotLight Lantern::spotLight() const {
    SpotLight spot;
    spot.position = m_heldPos;
    spot.direction = m_lightDir;
    spot.enabled = m_isOn && (m_oil > 0.0f);

    if (m_focused) {
        spot.innerCutoff = config::tune::LANTERN_FOCUS_INNER_CUTOFF;
        spot.outerCutoff = config::tune::LANTERN_FOCUS_OUTER_CUTOFF;
        spot.intensity = 2.4f * m_flickerIntensity;
        spot.range = 26.0f;
        spot.color = Vec3(1.0f, 0.92f, 0.75f);
    } else {
        spot.innerCutoff = config::tune::LANTERN_BASE_INNER_CUTOFF;
        spot.outerCutoff = config::tune::LANTERN_BASE_OUTER_CUTOFF;
        spot.intensity = 1.6f * m_flickerIntensity;
        spot.range = 17.0f;
        spot.color = Vec3(1.0f, 0.82f, 0.58f);
    }

    return spot;
}

} // namespace hm
