#include "BodyAudio.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace hm {

static inline float randf() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

BodyAudio::BodyAudio() {
}

void BodyAudio::update(float dt, float fearLevel, float staminaPercent, bool isInjured, bool isHoldingBreath, bool isListening) {
    m_fearLevel = clamp(fearLevel, 0.0f, 1.0f);
    m_staminaPercent = clamp(staminaPercent, 0.0f, 1.0f);
    m_isInjured = isInjured;
    m_isHoldingBreath = isHoldingBreath;
    m_isListening = isListening;

    if (m_tinnitusTimer > 0.0f) {
        m_tinnitusTimer -= dt;
        if (m_tinnitusTimer < 0.0f) m_tinnitusTimer = 0.0f;
    }

    // Target BPM: 65 (calm) to 165 (terrified)
    float targetBpm = 65.0f + m_fearLevel * 90.0f;
    if (m_staminaPercent < 0.3f) {
        targetBpm += 25.0f; // Winded
    }
    m_currentBpm += (targetBpm - m_currentBpm) * std::min(1.0f, dt * 1.5f);

    // Determine breathing state
    if (m_isHoldingBreath) {
        m_breathState = BreathState::HoldingBreath;
    } else if (m_fearLevel > 0.7f) {
        m_breathState = BreathState::Terrified;
    } else if (m_staminaPercent < 0.4f) {
        m_breathState = BreathState::Panting;
    } else {
        m_breathState = BreathState::Calm;
    }
}

void BodyAudio::triggerTinnitus(float durationSec) {
    m_tinnitusTimer = durationSec;
}

std::vector<int16_t> BodyAudio::generateFootstep(FootstepSurface surface, bool isRightFoot, bool isSprinting, bool isCrouching) {
    float stepLenSec = isSprinting ? 0.22f : (isCrouching ? 0.40f : 0.32f);
    int totalSamples = static_cast<int>(SAMPLE_RATE * stepLenSec);
    std::vector<int16_t> out(totalSamples);

    // Base pitch & character by surface
    float f1 = 120.0f;
    float noiseRatio = 0.4f;
    switch (surface) {
        case FootstepSurface::Wood:
            f1 = 140.0f; // Wood creak/resonance
            noiseRatio = 0.35f;
            break;
        case FootstepSurface::Stone:
            f1 = 95.0f;  // Solid stone thud
            noiseRatio = 0.50f;
            break;
        case FootstepSurface::Carpet:
            f1 = 80.0f;  // Muffled dull thud
            noiseRatio = 0.20f;
            break;
        case FootstepSurface::Metal:
            f1 = 450.0f; // Metallic clang
            noiseRatio = 0.60f;
            break;
        case FootstepSurface::Water:
            f1 = 280.0f; // Wet splash
            noiseRatio = 0.75f;
            break;
    }

    // Limp asymmetry: if injured and right foot, heavier drag
    float ampMult = 1.0f;
    if (m_isInjured && isRightFoot) {
        ampMult = 1.35f;
    } else if (m_isInjured && !isRightFoot) {
        ampMult = 0.75f;
    }

    if (isSprinting) ampMult *= 1.4f;
    if (isCrouching) ampMult *= 0.45f;

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float env = std::exp(-18.0f * t);

        // Transient thud + surface noise
        float thud = std::sin(2.0f * PI * f1 * t) * (1.0f - noiseRatio);
        float noise = (randf() * 2.0f - 1.0f) * noiseRatio;

        float s = (thud + noise) * env * ampMult;
        out[i] = static_cast<int16_t>(clamp(s * 22000.0f, -32767.0f, 32767.0f));
    }

    return out;
}

std::vector<int16_t> BodyAudio::renderBreathingChunk(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples, 0);

    if (m_breathState == BreathState::HoldingBreath) {
        return out; // Silenced while holding breath
    }

    float rateHz = 0.35f; // Calm: ~21 breaths/min
    float breathVol = 0.18f;
    if (m_breathState == BreathState::Panting) {
        rateHz = 0.95f; // Fast panting
        breathVol = 0.45f;
    } else if (m_breathState == BreathState::Terrified) {
        rateHz = 0.75f; // Shuddering gasp
        breathVol = 0.55f;
    }

    for (int i = 0; i < totalSamples; ++i) {
        m_breathPhase += 1.0f / static_cast<float>(SAMPLE_RATE);
        float t = m_breathPhase;

        float breathCycle = std::sin(2.0f * PI * rateHz * t);
        float breathEnv = std::abs(breathCycle);
        breathEnv = std::pow(breathEnv, 1.4f);

        // Filtered breath noise
        float noise = (randf() * 2.0f - 1.0f);
        float s = noise * breathEnv * breathVol;

        out[i] = static_cast<int16_t>(clamp(s * 18000.0f, -32767.0f, 32767.0f));
    }

    return out;
}

std::vector<int16_t> BodyAudio::renderHeartbeatChunk(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    float heartHz = m_currentBpm / 60.0f; // e.g., 1.5 Hz at 90 BPM

    for (int i = 0; i < totalSamples; ++i) {
        m_heartPhase += 1.0f / static_cast<float>(SAMPLE_RATE);
        float t = m_heartPhase;

        float cycle = std::fmod(t * heartHz, 1.0f); // 0 to 1 per beat
        float s = 0.0f;

        // "Lub" (first sound) at cycle = 0.0
        if (cycle < 0.12f) {
            float env = std::sin(PI * (cycle / 0.12f));
            s += std::sin(2.0f * PI * 58.0f * (cycle / heartHz)) * env * 0.8f;
        }
        // "Dub" (second sound) at cycle = 0.22
        else if (cycle > 0.20f && cycle < 0.32f) {
            float env = std::sin(PI * ((cycle - 0.20f) / 0.12f));
            s += std::sin(2.0f * PI * 68.0f * ((cycle - 0.20f) / heartHz)) * env * 0.65f;
        }

        // Low-pass muffle if injured or holding breath
        float vol = (m_fearLevel * 0.8f + 0.2f);
        if (m_isHoldingBreath) vol *= 1.35f; // Heartbeat rings loud in ears when holding breath!

        out[i] = static_cast<int16_t>(clamp(s * vol * 24000.0f, -32767.0f, 32767.0f));
    }

    return out;
}

std::vector<int16_t> BodyAudio::renderTinnitusChunk(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples, 0);

    if (m_tinnitusTimer <= 0.0f) return out;

    float fade = clamp(m_tinnitusTimer / 4.0f, 0.0f, 1.0f);
    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        // Pure 6.8 kHz sine wave ringing
        float s = std::sin(2.0f * PI * 6800.0f * t) * fade * 0.25f;
        out[i] = static_cast<int16_t>(clamp(s * 22000.0f, -32767.0f, 32767.0f));
    }

    return out;
}

} // namespace hm
