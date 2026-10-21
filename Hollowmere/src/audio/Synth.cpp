#include "Synth.h"
#include <cmath>
#include <algorithm>

namespace hm {

static constexpr float PI = 3.14159265358979323846f;
static constexpr float TWO_PI = 6.28318530717958647692f;

static inline float whiteNoise() {
    return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
}

std::vector<int16_t> Synth::generateFootstep(int variant) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * 0.22f);
    std::vector<int16_t> buf(totalSamples);

    float baseFreq = 70.0f + (variant % 3) * 15.0f;
    float lpf = 0.0f;

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float env = std::exp(-t * 26.0f);

        // Low thud
        float thud = std::sin(TWO_PI * baseFreq * t * (1.0f - t * 2.5f)) * env;

        // Wood creak noise
        float n = whiteNoise();
        lpf += (n - lpf) * 0.18f;
        float creak = lpf * std::exp(-t * 18.0f) * 0.5f;

        float sample = (thud * 0.65f + creak * 0.35f);
        buf[i] = static_cast<int16_t>(std::clamp(sample * 24000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

std::vector<int16_t> Synth::generateDoorCreak() {
    int totalSamples = static_cast<int>(SAMPLE_RATE * 0.85f);
    std::vector<int16_t> buf(totalSamples);

    float phase = 0.0f;
    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float env = std::sin(PI * (t / 0.85f));

        // Frequency sweep with jitter
        float freq = 180.0f + std::sin(t * 30.0f) * 80.0f + t * 90.0f;
        phase += TWO_PI * freq / SAMPLE_RATE;

        float saw = (std::fmod(phase, TWO_PI) / PI) - 1.0f;
        float friction = (whiteNoise() > 0.4f ? 0.3f : -0.3f);

        float sample = (saw * 0.6f + friction * 0.4f) * env;
        buf[i] = static_cast<int16_t>(std::clamp(sample * 16000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

std::vector<int16_t> Synth::generateDoorSlam() {
    int totalSamples = static_cast<int>(SAMPLE_RATE * 0.65f);
    std::vector<int16_t> buf(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float env = std::exp(-t * 9.0f);
        float boom = std::sin(TWO_PI * 52.0f * t * (1.0f - t)) * env;
        float impact = whiteNoise() * std::exp(-t * 35.0f);

        float sample = boom * 0.7f + impact * 0.5f;
        buf[i] = static_cast<int16_t>(std::clamp(sample * 28000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

std::vector<int16_t> Synth::generateHeartbeat(float bpm) {
    float beatDur = 60.0f / bpm;
    int totalSamples = static_cast<int>(SAMPLE_RATE * beatDur);
    std::vector<int16_t> buf(totalSamples, 0);

    auto addPulse = [&](float offsetTime, float freq, float amp) {
        int start = static_cast<int>(offsetTime * SAMPLE_RATE);
        int dur = static_cast<int>(SAMPLE_RATE * 0.16f);
        for (int i = 0; i < dur && (start + i) < totalSamples; ++i) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = std::exp(-t * 22.0f);
            float s = std::sin(TWO_PI * freq * t) * env * amp;
            int idx = start + i;
            buf[idx] += static_cast<int16_t>(std::clamp(s * 25000.0f, -32767.0f, 32767.0f));
        }
    };

    // Lub (0.0s) & Dub (0.18s)
    addPulse(0.0f, 48.0f, 0.9f);
    addPulse(0.18f, 58.0f, 0.75f);

    return buf;
}

std::vector<int16_t> Synth::generateBreathing(bool panting) {
    float dur = panting ? 1.0f : 2.5f;
    int totalSamples = static_cast<int>(SAMPLE_RATE * dur);
    std::vector<int16_t> buf(totalSamples);

    float lpf = 0.0f;
    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float env = std::sin(PI * (t / dur));
        float n = whiteNoise();
        lpf += (n - lpf) * 0.06f;

        float sample = lpf * env * (panting ? 0.7f : 0.35f);
        buf[i] = static_cast<int16_t>(std::clamp(sample * 22000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

std::vector<int16_t> Synth::generateAmbientDrone(float durationSeconds) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSeconds);
    std::vector<int16_t> buf(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;

        // Sub bass drone 55 Hz (A1) and 82.4 Hz (E2)
        float s1 = std::sin(TWO_PI * 55.0f * t);
        float s2 = std::sin(TWO_PI * 82.4f * t + 0.5f);
        float s3 = std::sin(TWO_PI * 110.0f * t + std::sin(t * 1.5f) * 0.8f) * 0.3f;

        // Slow atmospheric filter pulsation
        float lfo = 0.7f + 0.3f * std::sin(TWO_PI * 0.25f * t);
        float drone = (s1 * 0.5f + s2 * 0.35f + s3 * 0.2f) * lfo;

        buf[i] = static_cast<int16_t>(std::clamp(drone * 18000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

std::vector<int16_t> Synth::generateGhostWail(float durationSeconds) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSeconds);
    std::vector<int16_t> buf(totalSamples);

    float phase = 0.0f;
    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float env = std::sin(PI * (t / durationSeconds));

        // Vibrato & weeping sweep
        float freq = 340.0f + std::sin(t * 7.0f) * 45.0f + std::sin(t * 1.2f) * 60.0f;
        phase += TWO_PI * freq / SAMPLE_RATE;

        // Formant synthesis approximation
        float carrier = std::sin(phase);
        float modulator = std::sin(phase * 2.01f) * 0.5f;

        float sample = (carrier + modulator) * env * 0.65f;
        buf[i] = static_cast<int16_t>(std::clamp(sample * 21000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

std::vector<int16_t> Synth::generateLanternClick() {
    int totalSamples = static_cast<int>(SAMPLE_RATE * 0.08f);
    std::vector<int16_t> buf(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float click = (whiteNoise() * 0.6f + std::sin(TWO_PI * 1800.0f * t) * 0.4f) * std::exp(-t * 80.0f);
        buf[i] = static_cast<int16_t>(std::clamp(click * 26000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

std::vector<int16_t> Synth::generatePickupChime() {
    int totalSamples = static_cast<int>(SAMPLE_RATE * 0.55f);
    std::vector<int16_t> buf(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float bell1 = std::sin(TWO_PI * 880.0f * t) * std::exp(-t * 8.0f);
        float bell2 = std::sin(TWO_PI * 1320.0f * t) * std::exp(-t * 12.0f);
        float chime = bell1 * 0.6f + bell2 * 0.4f;
        buf[i] = static_cast<int16_t>(std::clamp(chime * 20000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

std::vector<int16_t> Synth::generateJumpscareStinger() {
    int totalSamples = static_cast<int>(SAMPLE_RATE * 1.4f);
    std::vector<int16_t> buf(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float env = std::exp(-t * 3.5f);

        // Screech cluster + sub drop
        float screech = (whiteNoise() * 0.5f +
                         std::sin(TWO_PI * 2200.0f * t + std::sin(t * 35.0f) * 5.0f) * 0.3f +
                         std::sin(TWO_PI * 2780.0f * t) * 0.2f);
        float subDrop = std::sin(TWO_PI * (120.0f - t * 65.0f) * t) * 0.6f;

        float sample = (screech * 0.7f + subDrop * 0.5f) * env;
        buf[i] = static_cast<int16_t>(std::clamp(sample * 30000.0f, -32767.0f, 32767.0f));
    }
    return buf;
}

} // namespace hm
