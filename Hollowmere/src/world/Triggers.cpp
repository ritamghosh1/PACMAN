#include "Triggers.h"
#include <cmath>
#include <algorithm>

namespace hm {

TriggerSystem::TriggerSystem() {
    initDefaultScareTriggers();
}

void TriggerSystem::initDefaultScareTriggers() {
    m_triggers.clear();
    // Wing 0 Foyer Scare Triggers
    m_triggers.push_back({Vec3(0.0f, 0.0f, 5.0f), 2.5f, TriggerType::LightFlicker, false, "The chandelier crystals violently shudder."});
    m_triggers.push_back({Vec3(4.0f, 0.0f, 8.0f), 2.2f, TriggerType::WhisperBehind, false, "A cold breath grazes the back of your neck."});
    m_triggers.push_back({Vec3(-4.0f, 0.0f, 12.0f), 2.0f, TriggerType::DoorSlam, false, "A heavy oak door slams in the distance."});

    // Wing 1 Library Scare Triggers
    m_triggers.push_back({Vec3(8.0f, 0.0f, 15.0f), 2.0f, TriggerType::ShadowPass, false, "A tall silhouette darts past the bookshelf slit."});
    m_triggers.push_back({Vec3(12.0f, 0.0f, 18.0f), 2.0f, TriggerType::PaintingFollow, false, "The painted eyes in the portrait snap toward you."});

    // Wing 2 Conservatory Scare Triggers
    m_triggers.push_back({Vec3(-6.0f, 0.0f, 16.0f), 2.2f, TriggerType::ChairMove, false, "A wrought-iron chair scrapes against the tile."});

    // Wing 4 Nursery Scare Triggers
    m_triggers.push_back({Vec3(-8.0f, 0.0f, -5.0f), 2.5f, TriggerType::DollFlicker, false, "A porcelain doll blinks from the nursery shelf."});
}

void TriggerSystem::reset() {
    for (auto& t : m_triggers) t.triggered = false;
    m_cooldown = 0.0f;
}

void TriggerSystem::update(float dt, const Vec3& playerPos, std::function<void(TriggerType, const ScareTrigger&)> onTrigger) {
    if (m_cooldown > 0.0f) {
        m_cooldown -= dt;
    }

    for (auto& trigger : m_triggers) {
        if (!trigger.triggered && m_cooldown <= 0.0f) {
            float distSq = (playerPos.x - trigger.position.x) * (playerPos.x - trigger.position.x) +
                           (playerPos.z - trigger.position.z) * (playerPos.z - trigger.position.z);
            if (distSq <= trigger.radius * trigger.radius) {
                trigger.triggered = true;
                m_cooldown = 20.0f; // Jumpscare budget cooldown: minimum 20s between soft scares
                if (onTrigger) {
                    onTrigger(trigger.type, trigger);
                }
                break;
            }
        }
    }
}

} // namespace hm
