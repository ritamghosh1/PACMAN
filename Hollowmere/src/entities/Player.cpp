#include "Player.h"
#include "core/config.h"
#include "core/Input.h"
#include <algorithm>
#include <cmath>

namespace hm {

Player::Player() = default;

Vec3 Player::forward() const {
    float yawRad = radians(m_yaw);
    return Vec3(std::cos(yawRad), 0.0f, std::sin(yawRad));
}

void Player::spawn(const Vec3& pos, float yawDeg) {
    m_position = pos;
    m_yaw = yawDeg;
    m_pitch = 0.0f;
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
    addRotation(input.mouseDeltaX() * m_mouseSens, (m_invertY ? 1.0f : -1.0f) * input.mouseDeltaY() * m_mouseSens);

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
    if (!m_interactionEnabled) return;

    float yawRad = radians(m_yaw);
    float pitchRad = radians(m_pitch);
    Vec3 fwd(std::cos(pitchRad) * std::cos(yawRad), std::sin(pitchRad), std::cos(pitchRad) * std::sin(yawRad));
    fwd = normalize(fwd);
    Vec3 eye = eyePosition();
    const bool pressed = input.wasKeyPressed(Qt::Key_E) || input.wasMouseButtonPressed(Qt::LeftButton);

    // 1. Pickups: forgiving "near and roughly facing" test instead of a pixel-perfect ray
    Pickup* best = nullptr;
    float bestScore = 0.0f;
    for (auto& p : pickups) {
        if (p.isCollected()) continue;
        Vec3 to = p.position() - eye;
        float d = length(to);
        if (d > config::tune::INTERACT_DISTANCE + 0.9f || d < 0.01f) continue;
        float facing = dot(to * (1.0f / d), fwd);
        if (facing < 0.55f && d > 1.3f) continue;
        float score = facing - d * 0.12f;
        if (!best || score > bestScore) { best = &p; bestScore = score; }
    }
    if (best) {
        const char* verb = best->type() == PickupType::Note ? "Read" : "Pick up";
        m_currentPrompt = QString("[E] %1 %2").arg(verb).arg(best->name());
        if (pressed) {
            best->collect();
            m_notificationToast = best->type() == PickupType::Note ? QString("Added to journal: %1").arg(best->name()) : QString("Found %1").arg(best->name());
            m_toastTimer = 3.0f;
            switch (best->type()) {
            case PickupType::OilFlask: m_lantern.addOil(40.0f); break;
            case PickupType::Key: m_keyFlags |= (1 << best->tag()); break;
            case PickupType::Bandage: m_bandageCount++; heal(); break;
            case PickupType::MemorySigil: m_sigilCount++; break;
            case PickupType::Note: break;
            }
            m_pickupEvents.push_back({best->type(), best->tag(), best->name(), best->description(), best->position()});
        }
        return;
    }

    // 2. Doors: nearest door in front of us (open doors have no collider, so no ray here)
    Door* bestDoor = nullptr;
    float bestD = 1e9f;
    for (auto& door : doors) {
        Vec3 to = door.position() + Vec3(0, 1.2f, 0) - eye;
        float d = length(to);
        if (d > config::tune::INTERACT_DISTANCE + 0.6f || d < 0.01f) continue;
        if (dot(to * (1.0f / d), fwd) < 0.45f) continue;
        if (d < bestD) { bestD = d; bestDoor = &door; }
    }
    if (bestDoor) {
        Door& door = *bestDoor;
        if (door.isLocked()) m_currentPrompt = hasKey(door.keyId()) ? QString("[E] Unlock door (%1)").arg(Door::keyName(door.keyId())) : QString("[E] Locked - needs the %1").arg(Door::keyName(door.keyId()));
        else m_currentPrompt = door.isOpen() ? "[E] Close door" : "[E] Open door";
        if (pressed && door.isOpen() && distance(Vec3(m_position.x, 0, m_position.z), Vec3(door.position().x, 0, door.position().z)) < 1.5f) {
            m_notificationToast = "You're standing in the doorway.";
            m_toastTimer = 1.8f;
        } else if (pressed) {
            QString msg;
            bool wasLocked = door.isLocked();
            if (door.tryInteract(m_keyFlags, msg)) {
                collision.setColliderEnabled(door.id(), !door.isOpen());
                m_doorEvents.push_back({wasLocked ? DoorEvent::Unlocked : (door.isOpen() ? DoorEvent::Opened : DoorEvent::Closed), door.position(), door.keyId()});
            } else {
                m_doorEvents.push_back({DoorEvent::Locked, door.position(), door.keyId()});
            }
            m_notificationToast = msg;
            m_toastTimer = 2.5f;
        }
    }
}

} // namespace hm
