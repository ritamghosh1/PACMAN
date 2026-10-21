#include "Camera.h"
#include <algorithm>

namespace hm {

Camera::Camera() = default;

void Camera::setRotation(float yawDeg, float pitchDeg) {
    m_yaw = yawDeg;
    m_pitch = clamp(pitchDeg, -89.0f, 89.0f);
}

void Camera::addRotation(float dYawDeg, float dPitchDeg) {
    m_yaw += dYawDeg;
    m_pitch = clamp(m_pitch + dPitchDeg, -89.0f, 89.0f);
}

void Camera::updateShake(float dt) {
    if (m_shakeTrauma > 0.0f) {
        m_shakeTrauma = std::max(0.0f, m_shakeTrauma - dt * 1.5f);
        m_shakeTime += dt * 35.0f;
        float intensity = m_shakeTrauma * m_shakeTrauma * 0.08f;
        m_shakeOffset = Vec3(
            std::sin(m_shakeTime * 1.3f) * intensity,
            std::cos(m_shakeTime * 1.7f) * intensity,
            std::sin(m_shakeTime * 0.9f) * (intensity * 0.5f)
        );
    } else {
        m_shakeOffset = Vec3(0, 0, 0);
    }
}

Vec3 Camera::forward() const {
    float yawRad = radians(m_yaw);
    float pitchRad = radians(m_pitch);

    Vec3 f;
    f.x = std::cos(pitchRad) * std::cos(yawRad);
    f.y = std::sin(pitchRad);
    f.z = std::cos(pitchRad) * std::sin(yawRad);
    return normalize(f);
}

Vec3 Camera::right() const {
    return normalize(cross(forward(), Vec3(0, 1, 0)));
}

Vec3 Camera::up() const {
    return normalize(cross(right(), forward()));
}

Mat4 Camera::viewMatrix() const {
    Vec3 eye = m_position + m_bobOffset + m_shakeOffset;
    Vec3 target = eye + forward();
    return Mat4::lookAt(eye, target, Vec3(0, 1, 0));
}

Mat4 Camera::projMatrix(float aspect) const {
    return Mat4::perspective(radians(m_fov), aspect, m_near, m_far);
}

} // namespace hm
