#pragma once

#include "core/mathx.h"

namespace hm {

class Camera {
public:
    Camera();

    void setPosition(const Vec3& p) { m_position = p; }
    const Vec3& position() const { return m_position; }

    void setRotation(float yawDeg, float pitchDeg);
    void addRotation(float dYawDeg, float dPitchDeg);

    float yaw() const { return m_yaw; }
    float pitch() const { return m_pitch; }

    void setFov(float fovDeg) { m_fov = fovDeg; }
    float fov() const { return m_fov; }

    void setClipPlanes(float nearP, float farP) { m_near = nearP; m_far = farP; }
    float nearPlane() const { return m_near; }
    float farPlane() const { return m_far; }

    void setBobOffset(const Vec3& offset) { m_bobOffset = offset; }
    const Vec3& bobOffset() const { return m_bobOffset; }

    void addShake(float amount) { m_shakeTrauma = std::min(1.0f, m_shakeTrauma + amount); }
    void updateShake(float dt);

    Vec3 forward() const;
    Vec3 right() const;
    Vec3 up() const;

    Mat4 viewMatrix() const;
    Mat4 projMatrix(float aspect) const;

private:
    Vec3 m_position{0.0f, 1.65f, 0.0f};
    Vec3 m_bobOffset{0.0f, 0.0f, 0.0f};
    Vec3 m_shakeOffset{0.0f, 0.0f, 0.0f};

    float m_yaw = -90.0f; // facing -Z by default
    float m_pitch = 0.0f;
    float m_fov = 75.0f;
    float m_near = 0.05f;
    float m_far = 120.0f;

    float m_shakeTrauma = 0.0f;
    float m_shakeTime = 0.0f;
};

} // namespace hm
