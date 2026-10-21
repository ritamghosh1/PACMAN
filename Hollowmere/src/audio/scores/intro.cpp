#include "motifs.h"
#include "audio/Synth2.h"
#include "core/mathx.h"
#include <cmath>

namespace hm {
namespace scores {

std::vector<int16_t> renderIntroScore() {
    // 80-second cinematic progression:
    // 0-20s: Distant thunder, quiet rain bed, solitary felt piano chord
    // 20-45s: Low bowed cello drone starts, music-box motif emerges faint & fractured
    // 45-65s: Rising strings, dissonance crescendo, heartbeat accelerating
    // 65-75s: Climax chord as the manor gates slam shut, lightning strike
    // 75-80s: Sudden dead silence with decaying reverb tail
    const float totalSec = 80.0f;
    int totalSamples = static_cast<int>(Synth2::SAMPLE_RATE * totalSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(Synth2::SAMPLE_RATE);
        float s = 0.0f;

        // Low pedal tone D1 (36.7 Hz) throughout
        float pedal = std::sin(2.0f * PI * 36.7f * t) * 0.3f;
        s += pedal;

        // 20-65s: Cello drone D2 (73.4 Hz) + F2 (87.3 Hz)
        if (t > 15.0f && t < 70.0f) {
            float fade = 1.0f;
            if (t < 25.0f) fade = (t - 15.0f) / 10.0f;
            else if (t > 65.0f) fade = (70.0f - t) / 5.0f;

            float cello = std::sin(2.0f * PI * 73.42f * t) + 0.4f * std::sin(2.0f * PI * 87.31f * t);
            s += cello * fade * 0.35f;
        }

        // 30-55s: Fragment of music-box motif
        if (t > 30.0f && t < 55.0f) {
            float mt = t - 30.0f;
            float noteFreq = (std::fmod(mt, 3.0f) < 1.5f) ? 293.66f : 349.23f;
            float noteEnv = std::exp(-2.0f * std::fmod(mt, 1.5f));
            float mb = std::sin(2.0f * PI * noteFreq * t) * noteEnv;
            s += mb * 0.25f;
        }

        // 55-68s: Accelerating heartbeat and dissonant string riser
        if (t >= 55.0f && t < 68.0f) {
            float progress = (t - 55.0f) / 13.0f;
            float riser = std::sin(2.0f * PI * (110.0f + 250.0f * progress * progress) * t);
            s += riser * progress * 0.5f;

            // Heartbeat thump
            float beat = std::abs(std::sin(2.0f * PI * (1.2f + 1.5f * progress) * t));
            beat = std::pow(beat, 6.0f);
            s += std::sin(2.0f * PI * 60.0f * t) * beat * 0.4f;
        }

        // 68s: Gate slam impact & thunder stinger
        if (t >= 68.0f && t < 74.0f) {
            float st = t - 68.0f;
            float impact = std::sin(2.0f * PI * 45.0f * st) * std::exp(-2.0f * st);
            s += impact * 0.8f;
        }

        // 74-80s: Fade to dead silence
        if (t >= 74.0f) {
            float tail = clamp((80.0f - t) / 6.0f, 0.0f, 1.0f);
            s *= tail;
        }

        out[i] = static_cast<int16_t>(clamp(s * 20000.0f, -32767.0f, 32767.0f));
    }

    return out;
}

} // namespace scores
} // namespace hm
