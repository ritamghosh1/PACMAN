#pragma once

#include "core/mathx.h"
#include "Lantern.h"
#include "world/Collision.h"
#include "world/Door.h"
#include "world/Pickup.h"
#include <vector>
#include <QString>

namespace hm {

enum class HealthState {
    Healthy,
    Wounded,
    Critical,
    Dead
};

class Player {
public:
    Player();

    void spawn(const Vec3& pos);
    void update(float dt, Collision& collision, std::vector<Door>& doors, std::vector<Pickup>& pickups);

    const Vec3& position() const { return m_position; }
    Vec3 eyePosition() const;

    float yaw() const { return m_yaw; }
    float pitch() const { return m_pitch; }
    void addRotation(float dYaw, float dPitch);

    float stamina() const { return m_stamina; }
    float staminaRatio() const;

    HealthState healthState() const { return m_healthState; }
    float healthNormalized() const;
    void heal();
    void restoreHealth() { heal(); }
    void takeDamage();

    float sanity() const { return m_sanity; }
    float sanityNormalized() const { return m_sanity / 100.0f; }
    void restoreSanity(float s) { m_sanity = clamp(m_sanity + s, 0.0f, 100.0f); }
    void setSanity(float s) { m_sanity = clamp(s, 0.0f, 100.0f); }
    void refillOil(float amount) { m_lantern.addOil(amount); }
    int sigilsCollected() const { return m_sigilCount; }
    void collectSigil() { m_sigilCount++; }
    bool hasBrassKey() const { return (m_keyFlags & 1) != 0; }

    Lantern& lantern() { return m_lantern; }
    const Lantern& lantern() const { return m_lantern; }

    bool isMoving() const { return m_isMoving; }
    bool isSprinting() const { return m_isSprinting; }
    bool isCrouching() const { return m_isCrouching; }

    // Footstep audio event trigger
    bool checkFootstepTriggered();

    // Interaction
    const QString& currentPrompt() const { return m_currentPrompt; }
    const QString& notificationToast() const { return m_notificationToast; }
    float toastTimer() const { return m_toastTimer; }

    int keyFlags() const { return m_keyFlags; }
    int sigilCount() const { return m_sigilCount; }
    int bandageCount() const { return m_bandageCount; }

private:
    void handleInteraction(Collision& collision, std::vector<Door>& doors, std::vector<Pickup>& pickups);

    Vec3 m_position{0, 0, 0};
    Vec3 m_velocity{0, 0, 0};

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;

    float m_currentEyeHeight = 1.65f;
    float m_targetEyeHeight = 1.65f;

    float m_stamina = 6.0f;
    bool m_isSprinting = false;
    bool m_isCrouching = false;
    bool m_isMoving = false;

    // Head bobbing
    float m_bobCycle = 0.0f;
    Vec3 m_bobOffset{0, 0, 0};
    bool m_footstepTriggered = false;

    HealthState m_healthState = HealthState::Healthy;
    float m_sanity = 100.0f;

    Lantern m_lantern;

    // Inventory
    int m_keyFlags = 0;
    int m_sigilCount = 0;
    int m_bandageCount = 0;

    QString m_currentPrompt;
    QString m_notificationToast;
    float m_toastTimer = 0.0f;
};

} // namespace hm
