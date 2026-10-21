#include "MusicDirector.h"
#include "Synth2.h"
#include "core/mathx.h"
#include <cmath>
#include <algorithm>

namespace hm {

MusicDirector::MusicDirector() {
    setState(MusicState::DreadBed);
}

void MusicDirector::setIntensity(float intensity) {
    m_targetIntensity = clamp(intensity, 0.0f, 1.0f);
}

void MusicDirector::setState(MusicState state) {
    m_state = state;
}

void MusicDirector::triggerSilenceTrap(float durationSec) {
    m_silenceTrapTimer = durationSec;
}

void MusicDirector::update(float dt) {
    if (m_silenceTrapTimer > 0.0f) {
        m_silenceTrapTimer -= dt;
        if (m_silenceTrapTimer < 0.0f) m_silenceTrapTimer = 0.0f;
    }

    // Smooth intensity interpolation
    m_intensity += (m_targetIntensity - m_intensity) * std::min(1.0f, dt * 1.5f);

    // Compute stem levels
    if (m_silenceTrapTimer > 0.0f || m_state == MusicState::Silence) {
        m_stemL0 = 0.0f;
        m_stemL1 = 0.0f;
        m_stemL2 = 0.0f;
        m_stemL3 = 0.0f;
        return;
    }

    if (m_state == MusicState::SafeRoom) {
        m_stemL0 = 0.35f;
        m_stemL1 = 0.15f;
        m_stemL2 = 0.0f;
        m_stemL3 = 0.0f;
        return;
    }

    if (m_state == MusicState::Hiding) {
        m_stemL0 = 0.4f;
        m_stemL1 = 0.2f;
        m_stemL2 = 0.7f; // Forward heartbeat
        m_stemL3 = 0.0f;
        return;
    }

    // Dread / Pursuit state driven by intensity:
    // L0: always on (0.5 to 1.0)
    // L1: rises at intensity > 0.2
    // L2: rises at intensity > 0.45
    // L3: rises at intensity > 0.75
    m_stemL0 = 0.6f + 0.4f * (1.0f - m_intensity);
    m_stemL1 = clamp((m_intensity - 0.2f) / 0.35f, 0.0f, 1.0f);
    m_stemL2 = clamp((m_intensity - 0.45f) / 0.35f, 0.0f, 1.0f);
    m_stemL3 = clamp((m_intensity - 0.75f) / 0.25f, 0.0f, 1.0f);
}

std::vector<int16_t> MusicDirector::renderChunk(float durationSec) {
    int totalSamples = static_cast<int>(Synth2::SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples, 0);

    if (m_silenceTrapTimer > 0.0f || m_state == MusicState::Silence) {
        return out; // Pure dead silence
    }

    for (int i = 0; i < totalSamples; ++i) {
        m_scorePhase += 1.0f / static_cast<float>(Synth2::SAMPLE_RATE);
        float t = m_scorePhase;

        float s = 0.0f;

        // Stem L0: Sub-bass 44 Hz drone with subtle slow beat
        if (m_stemL0 > 0.01f) {
            float l0 = std::sin(2.0f * PI * 44.0f * t) + 0.3f * std::sin(2.0f * PI * 88.0f * t);
            s += l0 * m_stemL0 * 0.35f;
        }

        // Stem L1: Bowed D minor / Phrygian drone pad (73.4 Hz + 110 Hz)
        if (m_stemL1 > 0.01f) {
            float l1 = std::sin(2.0f * PI * 73.42f * t) + 0.5f * std::sin(2.0f * PI * 110.0f * t);
            s += l1 * m_stemL1 * 0.30f;
        }

        // Stem L2: Tense pulse (BPM 120 rhythmic heartbeat pulse)
        if (m_stemL2 > 0.01f) {
            float pulse = std::abs(std::sin(2.0f * PI * 2.0f * t));
            pulse = std::pow(pulse, 8.0f);
            float pulseWave = std::sin(2.0f * PI * 65.0f * t) * pulse;
            s += pulseWave * m_stemL2 * 0.45f;
        }

        // Stem L3: Pursuit tritone dissonance (C-F# ostinato + harsh saw)
        if (m_stemL3 > 0.01f) {
            float tritone = std::sin(2.0f * PI * 130.81f * t) + std::sin(2.0f * PI * 185.0f * t);
            s += tritone * m_stemL3 * 0.40f;
        }

        // Safe room lullaby overlay if in SafeRoom
        if (m_state == MusicState::SafeRoom) {
            float lullabyTone = std::sin(2.0f * PI * 293.66f * t) * std::exp(-2.0f * std::fmod(t, 1.2f));
            s += lullabyTone * 0.25f;
        }

        out[i] = static_cast<int16_t>(clamp(s * 22000.0f, -32767.0f, 32767.0f));
    }

    return out;
}

} // namespace hm
