#pragma once

#include "core/mathx.h"
#include <QString>
#include <vector>
#include <functional>

namespace hm {

enum class TriggerType {
    DoorSlam,
    ShadowPass,
    WhisperBehind,
    LightFlicker,
    PaintingFollow,
    ChairMove,
    DollFlicker
};

struct ScareTrigger {
    Vec3 position;
    float radius = 2.0f;
    TriggerType type;
    bool triggered = false;
    QString cueText;
};

class TriggerSystem {
public:
    TriggerSystem();

    void initDefaultScareTriggers();
    void update(float dt, const Vec3& playerPos, std::function<void(TriggerType, const ScareTrigger&)> onTrigger);

    void reset();

private:
    std::vector<ScareTrigger> m_triggers;
    float m_cooldown = 0.0f;
};

} // namespace hm
