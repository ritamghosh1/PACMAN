#pragma once

#include "core/mathx.h"

namespace hm {

class SanitySystem {
public:
    SanitySystem();

    void update(float dt,
                bool inDarkness,
                bool nearGhost,
                bool inSafeRoom,
                bool isListening,
                bool isHumming);

    float sanity() const { return m_sanity; }
    void setSanity(float val) { m_sanity = clamp(val, 0.0f, 100.0f); }

    void triggerGoreExposure() { m_sanity = std::max(0.0f, m_sanity - 12.0f); }
    void addPhotoReward() { m_sanity = std::min(100.0f, m_sanity + 25.0f); }

    // Progressive hallucination flags
    bool hasPeripheralShadows() const { return m_sanity < 75.0f; }
    bool hasPhantomWhispers() const { return m_sanity < 60.0f; }
    bool hasFakeFootsteps() const { return m_sanity < 45.0f; }
    bool hasHallucinationGhost() const { return m_sanity < 25.0f; }
    bool isZeroSanityStalkerActive() const { return m_sanity <= 0.01f; }

    float desaturationAmount() const; // 0.0 to 0.7 for PostFx
    float chromaticAberration() const;

private:
    float m_sanity = 100.0f;
};

} // namespace hm
