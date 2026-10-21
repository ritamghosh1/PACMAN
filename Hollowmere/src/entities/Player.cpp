#include "Player.h"
#include "core/config.h"
#include "core/Input.h"
#include <algorithm>
#include <cmath>

namespace hm {

Player::Player() = default;

void Player::spawn(const Vec3& pos) {
    m_position = pos;
    m_velocity = Vec3(0, 0, 0);
    m_stamina = config::tune::STAMINA_MAX;
    m_healthState = HealthState::Healthy;
    m_sanity = 100.0f;
    m_currentEyeHeight = config::tune::STAND_EYE_HEIGHT;
    m_targetEyeHeight = config::tune::STAND_EYE_HEIGHT;
}

void Player::addRotation(float dYaw, float dPitch) {
    m_yaw += dYaw;
    m_pitch = clamp(m_pitch + dPitch, -89.0f, 89.0f);
}

float Player::staminaRatio() const {
    return clamp(m_stamina / config::tune::STAMINA_MAX, 0.0f, 1.0f);
}

float Player::healthNormalized() const {
    switch (m_healthState) {
        case HealthState::Healthy: return 1.0f;
        case HealthState::Wounded: return 0.5f;
        case HealthState::Critical: return 0.2f;
        case HealthState::Dead: return 0.0f;
    }
    return 1.0f;
}

void Player::heal() {
    if (m_healthState == HealthState::Wounded) {
        m_healthState = HealthState::Healthy;
    } else if (m_healthState == HealthState::Critical) {
        m_healthState = HealthState::Wounded;
    }
}

void Player::takeDamage() {
    if (m_healthState == HealthState::Healthy) {
        m_healthState = HealthState::Wounded;
    } else if (m_healthState == HealthState::Wounded) {
        m_healthState = HealthState::Critical;
    } else if (m_healthState == HealthState::Critical) {
        m_healthState = HealthState::Dead;
    }
}

Vec3 Player::eyePosition() const {
    return m_position + Vec3(0, m_currentEyeHeight, 0) + m_bobOffset;
}

bool Player::checkFootstepTriggered() {
    if (m_footstepTriggered) {
        m_footstepTriggered = false;
        return true;
    }
    return false;
}

void Player::update(float dt, Collision& collision, std::vector<Door>& doors, std::vector<Pickup>& pickups) {
    Input& input = Input::instance();

    // Mouse look
    float mouseSens = 0.12f;
    addRotation(input.mouseDeltaX() * mouseSens, -input.mouseDeltaY() * mouseSens);

    // Flashlight controls
    if (input.wasKeyPressed(Qt::Key_F)) {
        m_lantern.toggle();
    }
    m_lantern.setFocused(input.isMouseButtonDown(Qt::RightButton));

    // Crouch handling
    m_isCrouching = input.isKeyDown(Qt::Key_C) || input.isKeyDown(Qt::Key_Control);
    m_targetEyeHeight = m_isCrouching ? config::tune::CROUCH_EYE_HEIGHT : config::tune::STAND_EYE_HEIGHT;
    m_currentEyeHeight += (m_targetEyeHeight - m_currentEyeHeight) * std::min(1.0f, dt * 10.0f);

    // Movement direction vectors
    float yawRad = radians(m_yaw);
    Vec3 fwd(std::cos(yawRad), 0.0f, std::sin(yawRad));
    fwd = normalize(fwd);
    Vec3 right(-fwd.z, 0.0f, fwd.x);

    Vec3 moveDir(0, 0, 0);
    if (input.isKeyDown(Qt::Key_W)) moveDir += fwd;
    if (input.isKeyDown(Qt::Key_S)) moveDir -= fwd;
    if (input.isKeyDown(Qt::Key_D)) moveDir += right;
    if (input.isKeyDown(Qt::Key_A)) moveDir -= right;

    float moveLen = length(moveDir);
    m_isMoving = moveLen > 0.01f;

    // Sprinting & Stamina
    bool wantSprint = input.isKeyDown(Qt::Key_Shift) && input.isKeyDown(Qt::Key_W) && !m_isCrouching;
    if (wantSprint && m_stamina > 0.2f) {
        m_isSprinting = true;
        m_stamina = std::max(0.0f, m_stamina - config::tune::STAMINA_DRAIN_RATE * dt);
    } else {
        m_isSprinting = false;
        m_stamina = std::min(config::tune::STAMINA_MAX, m_stamina + config::tune::STAMINA_RECOVERY_RATE * dt);
    }

    // Speed selection
    float currentSpeed = config::tune::WALK_SPEED;
    if (m_isCrouching) {
        currentSpeed = config::tune::CROUCH_SPEED;
    } else if (m_isSprinting) {
        currentSpeed = config::tune::SPRINT_SPEED;
    }

    // Critical health reduces speed
    if (m_healthState == HealthState::Critical) {
        currentSpeed *= 0.7f;
    }

    // Apply movement & collision resolution
    if (m_isMoving) {
        moveDir = normalize(moveDir);
        Vec3 targetPos = m_position + moveDir * (currentSpeed * dt);
        m_position = collision.resolveMotion(m_position, targetPos, config::tune::PLAYER_RADIUS, 1.8f);
    }

    // Head bob & footstep timing
    if (m_isMoving) {
        float bobFreq = m_isSprinting ? 12.0f : (m_isCrouching ? 6.5f : 8.5f);
        float prevCycle = m_bobCycle;
        m_bobCycle += dt * bobFreq;

        float bobAmp = m_isSprinting ? 0.065f : (m_isCrouching ? 0.02f : 0.04f);
        m_bobOffset.y = std::sin(m_bobCycle) * bobAmp;
        m_bobOffset.x = std::cos(m_bobCycle * 0.5f) * (bobAmp * 0.6f);

        // Footstep event when sine passes minimum
        if (std::sin(prevCycle) >= -0.9f && std::sin(m_bobCycle) < -0.9f) {
            m_footstepTriggered = true;
        }
    } else {
        m_bobOffset = m_bobOffset * std::max(0.0f, 1.0f - dt * 10.0f);
        m_bobCycle = 0.0f;
    }

    // Update Lantern
    float pitchRad = radians(m_pitch);
    Vec3 fullCamFwd(std::cos(pitchRad) * std::cos(yawRad), std::sin(pitchRad), std::cos(pitchRad) * std::sin(yawRad));
    fullCamFwd = normalize(fullCamFwd);

    m_lantern.update(dt, eyePosition(), fullCamFwd, right, m_isMoving);

    // Notification toast timer
    if (m_toastTimer > 0.0f) {
        m_toastTimer -= dt;
    }

    // Handle Interaction raycast
    handleInteraction(collision, doors, pickups);
}

void Player::handleInteraction(Collision& collision, std::vector<Door>& doors, std::vector<Pickup>& pickups) {
    Input& input = Input::instance();
    m_currentPrompt.clear();

    float yawRad = radians(m_yaw);
    float pitchRad = radians(m_pitch);
    Vec3 fwd(std::cos(pitchRad) * std::cos(yawRad), std::sin(pitchRad), std::cos(pitchRad) * std::sin(yawRad));
    fwd = normalize(fwd);

    Ray interactRay(eyePosition(), fwd);

    // 1. Check Pickups
    for (auto& p : pickups) {
        if (p.isCollected()) continue;
        float hitDist = 0.0f;
        if (interactRay.intersectAABB(p.bounds(), hitDist) && hitDist < config::tune::INTERACT_DISTANCE) {
            m_currentPrompt = QString("[E] Pick up %1").arg(p.name());

            if (input.wasKeyPressed(Qt::Key_E) || input.wasMouseButtonPressed(Qt::LeftButton)) {
                p.collect();
                m_notificationToast = QString("Found %1").arg(p.name());
                m_toastTimer = 3.0f;

                if (p.type() == PickupType::OilFlask) {
                    m_lantern.addOil(35.0f);
                } else if (p.type() == PickupType::BrassKey) {
                    m_keyFlags |= (1 << 1);
                } else if (p.type() == PickupType::Bandage) {
                    m_bandageCount++;
                    heal();
                } else if (p.type() == PickupType::MemorySigil) {
                    m_sigilCount++;
                }
            }
            return;
        }
    }

    // 2. Check Doors
    int hitTag = 0;
    int hitId = 0;
    float hitDist = 0.0f;
    if (collision.raycast(interactRay, config::tune::INTERACT_DISTANCE, hitTag, hitId, hitDist)) {
        if (hitTag == 1) { // Door
            for (auto& door : doors) {
                if (door.id() == hitId) {
                    if (door.isLocked()) {
                        m_currentPrompt = "[E] Unlock Door (Locked)";
                    } else if (door.isOpen()) {
                        m_currentPrompt = "[E] Close Door";
                    } else {
                        m_currentPrompt = "[E] Open Door";
                    }

                    if (input.wasKeyPressed(Qt::Key_E) || input.wasMouseButtonPressed(Qt::LeftButton)) {
                        QString msg;
                        if (door.tryInteract(m_keyFlags, msg)) {
                            collision.setColliderEnabled(door.id(), !door.isOpen());
                        }
                        m_notificationToast = msg;
                        m_toastTimer = 2.5f;
                    }
                    return;
                }
            }
        }
    }
}

} // namespace hm
