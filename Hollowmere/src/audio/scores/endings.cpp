#include "motifs.h"
#include "audio/Synth2.h"
#include "core/mathx.h"
#include <cmath>

namespace hm {
namespace scores {

std::vector<int16_t> renderEndingScoreE1() {
    // E1: Acceptance & Light - Resolves to warm D Major (D, F#, A)
    const float durationSec = 16.0f;
    int totalSamples = static_cast<int>(Synth2::SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(Synth2::SAMPLE_RATE);

        // Major triad: D3 (146.8 Hz), F#3 (185.0 Hz), A3 (220.0 Hz)
        float s = 0.4f * std::sin(2.0f * PI * 146.83f * t);
        s += 0.35f * std::sin(2.0f * PI * 184.99f * t); // F#3 Major 3rd
        s += 0.30f * std::sin(2.0f * PI * 220.00f * t);

        // High shimmer
        s += 0.15f * std::sin(2.0f * PI * 587.33f * t);

        float env = 1.0f;
        if (t < 2.0f) env = t / 2.0f;
        else if (t > durationSec - 3.0f) env = (durationSec - t) / 3.0f;

        out[i] = static_cast<int16_t>(clamp(s * env * 22000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> renderEndingScoreE2() {
    // E2: Consumed - Deep sub-bass descending tritone, final fading heartbeat
    const float durationSec = 12.0f;
    int totalSamples = static_cast<int>(Synth2::SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(Synth2::SAMPLE_RATE);
        float pitch = 55.0f - (t / durationSec) * 25.0f;
        float s = std::sin(2.0f * PI * pitch * t) * 0.6f;

        // Fading heartbeat thumps
        float beat = std::abs(std::sin(2.0f * PI * 0.8f * t));
        beat = std::pow(beat, 10.0f) * clamp((durationSec - t) / durationSec, 0.0f, 1.0f);
        s += std::sin(2.0f * PI * 48.0f * t) * beat * 0.5f;

        out[i] = static_cast<int16_t>(clamp(s * 22000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> renderEndingScoreE3() {
    // E3: Trapped in the Loop - Music box starts again, eerie repetition
    return renderHushLittleLight(56.0f);
}

std::vector<int16_t> renderEndingScoreE4() {
    // E4: Mercy Harmonization - All 5 voices in peaceful ethereal chord
    const float durationSec = 14.0f;
    int totalSamples = static_cast<int>(Synth2::SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(Synth2::SAMPLE_RATE);
        // Harmonic open 5th chord: D2, A2, D3, A3, D4
        float s = 0.3f * std::sin(2.0f * PI * 73.42f * t);
        s += 0.25f * std::sin(2.0f * PI * 110.00f * t);
        s += 0.25f * std::sin(2.0f * PI * 146.83f * t);
        s += 0.20f * std::sin(2.0f * PI * 220.00f * t);
        s += 0.15f * std::sin(2.0f * PI * 293.66f * t);

        float env = 1.0f;
        if (t < 2.5f) env = t / 2.5f;
        else if (t > durationSec - 2.5f) env = (durationSec - t) / 2.5f;

        out[i] = static_cast<int16_t>(clamp(s * env * 22000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> renderEndingScoreE5() {
    // E5: The Warden's Truth - Low distorted cello drone with reverse melody
    const float durationSec = 12.0f;
    int totalSamples = static_cast<int>(Synth2::SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(Synth2::SAMPLE_RATE);
        float s = std::sin(2.0f * PI * 58.27f * t); // Bb1 dark Phrygian root
        s += 0.4f * std::sin(2.0f * PI * 116.54f * t);
        s = std::tanh(s * 2.2f) * 0.7f; // Overdrive distortion

        out[i] = static_cast<int16_t>(clamp(s * 22000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

} // namespace scores
} // namespace hm
