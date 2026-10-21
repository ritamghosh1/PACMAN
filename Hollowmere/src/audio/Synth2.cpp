#include "Synth2.h"
#include "core/mathx.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace hm {

// Helper pseudo-random float [0, 1)
static inline float randf() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

std::vector<int16_t> Synth2::generateMusicBoxNote(float freqHz, float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    // Inharmonic bell partials: f, 2.756f, 5.404f, 8.933f
    const float partialRatios[4] = {1.0f, 2.756f, 5.404f, 8.933f};
    const float partialAmps[4] = {0.65f, 0.28f, 0.12f, 0.05f};
    const float decayRates[4] = {1.8f, 3.2f, 5.5f, 9.0f};

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float sampleVal = 0.0f;

        for (int p = 0; p < 4; ++p) {
            float f = freqHz * partialRatios[p];
            float decay = std::exp(-decayRates[p] * t);
            sampleVal += partialAmps[p] * std::sin(2.0f * PI * f * t) * decay;
        }

        // Tiny metal tine attack click
        if (t < 0.004f) {
            sampleVal += (1.0f - t / 0.004f) * (randf() * 2.0f - 1.0f) * 0.15f;
        }

        out[i] = static_cast<int16_t>(clamp(sampleVal * 24000.0f, -32767.0f, 32767.0f));
    }

    return out;
}

std::vector<int16_t> Synth2::generateMusicBoxMotif() {
    // "Hush Little Light" 8 bars 3/4 D minor:
    // D4 (293.66), F4 (349.23), A4 (440.0), G4 (392.0), F4 (349.23), E4 (329.63), D4 (293.66), C#4 (277.18), D4 (293.66)
    const float notes[] = {293.66f, 349.23f, 440.0f, 392.0f, 349.23f, 329.63f, 293.66f, 277.18f, 293.66f};
    const float noteStepSec = 0.65f;
    const float noteDuration = 2.2f;

    int totalSamples = static_cast<int>(SAMPLE_RATE * (9 * noteStepSec + 2.0f));
    std::vector<float> mixBuffer(totalSamples, 0.0f);

    for (int n = 0; n < 9; ++n) {
        auto noteWave = generateMusicBoxNote(notes[n], noteDuration);
        int startSample = static_cast<int>(n * noteStepSec * SAMPLE_RATE);

        for (size_t i = 0; i < noteWave.size(); ++i) {
            if (startSample + i < mixBuffer.size()) {
                mixBuffer[startSample + i] += static_cast<float>(noteWave[i]);
            }
        }
    }

    std::vector<int16_t> result(totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        result[i] = static_cast<int16_t>(clamp(mixBuffer[i] * 0.75f, -32767.0f, 32767.0f));
    }
    return result;
}

std::vector<int16_t> Synth2::generateFeltPianoNote(float freqHz, float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float env = std::exp(-2.2f * t);

        // Warm harmonic series with felt damping
        float s = 0.6f * std::sin(2.0f * PI * freqHz * t) * env;
        s += 0.25f * std::sin(2.0f * PI * (freqHz * 2.0f) * t) * std::exp(-3.5f * t);
        s += 0.10f * std::sin(2.0f * PI * (freqHz * 3.0f) * t) * std::exp(-5.0f * t);
        s += 0.04f * std::sin(2.0f * PI * (freqHz * 4.0f) * t) * std::exp(-7.0f * t);

        // Soft hammer thud
        if (t < 0.015f) {
            float thud = std::sin(2.0f * PI * 65.0f * t) * (1.0f - t / 0.015f);
            s += thud * 0.3f;
        }

        out[i] = static_cast<int16_t>(clamp(s * 26000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateBowedCelloDrone(float baseFreqHz, float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    float phase = 0.0f;
    float lpf = 0.0f;

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);

        // Slow 5.2 Hz vibrato
        float vibrato = std::sin(2.0f * PI * 5.2f * t) * (baseFreqHz * 0.015f);
        float currentFreq = baseFreqHz + vibrato;

        // Bandlimited sawtooth
        phase += currentFreq / static_cast<float>(SAMPLE_RATE);
        if (phase >= 1.0f) phase -= 1.0f;
        float saw = 2.0f * phase - 1.0f;

        // Subtle bow rosin friction noise
        float bowNoise = (randf() * 2.0f - 1.0f) * 0.08f;
        float raw = saw + bowNoise;

        // Lowpass filter ~ 450 Hz
        lpf += 0.07f * (raw - lpf);

        // Seamless envelope fade in and fade out
        float env = 1.0f;
        if (t < 0.4f) env = t / 0.4f;
        else if (t > durationSec - 0.4f) env = (durationSec - t) / 0.4f;

        out[i] = static_cast<int16_t>(clamp(lpf * env * 22000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateDissonantStringRiser(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    float phase1 = 0.0f, phase2 = 0.0f, phase3 = 0.0f;
    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float progress = t / durationSec; // 0 to 1

        // Pitch sweeps up exponentially with dissonance
        float f1 = 110.0f * std::pow(3.2f, progress);
        float f2 = 116.54f * std::pow(3.3f, progress); // Minor 2nd dissonance
        float f3 = 155.56f * std::pow(3.1f, progress); // Tritone clash

        phase1 += f1 / static_cast<float>(SAMPLE_RATE);
        phase2 += f2 / static_cast<float>(SAMPLE_RATE);
        phase3 += f3 / static_cast<float>(SAMPLE_RATE);
        if (phase1 >= 1.0f) phase1 -= 1.0f;
        if (phase2 >= 1.0f) phase2 -= 1.0f;
        if (phase3 >= 1.0f) phase3 -= 1.0f;

        float s = (2.0f * phase1 - 1.0f) + (2.0f * phase2 - 1.0f) * 0.9f + (2.0f * phase3 - 1.0f) * 0.8f;
        float amp = progress * progress * 0.85f; // Exponential crescendo

        out[i] = static_cast<int16_t>(clamp(s * amp * 18000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateSubBassHit(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        // Pitch drop: 90Hz -> 38Hz
        float f = 38.0f + 52.0f * std::exp(-6.0f * t);
        float env = std::exp(-2.5f * t);
        float s = std::sin(2.0f * PI * f * t) * env;

        out[i] = static_cast<int16_t>(clamp(s * 28000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateGlassHarmonicaChord(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    // D minor cluster: D5 (587.33), F5 (698.46), A5 (880.0)
    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float s = 0.45f * std::sin(2.0f * PI * 587.33f * t);
        s += 0.35f * std::sin(2.0f * PI * 698.46f * t);
        s += 0.25f * std::sin(2.0f * PI * 880.00f * t);

        // Subtle shimmering chorus
        s += 0.15f * std::sin(2.0f * PI * 589.5f * t);

        float env = 1.0f;
        if (t < 0.5f) env = t / 0.5f;
        else if (t > durationSec - 0.5f) env = (durationSec - t) / 0.5f;

        out[i] = static_cast<int16_t>(clamp(s * env * 20000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateWeeperSobbing(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);

        // Quavering vocal tremor
        float tremor = std::sin(2.0f * PI * 6.5f * t) * 25.0f;
        // Sobbing pitch envelope
        float baseFreq = 260.0f + 70.0f * std::sin(2.0f * PI * 0.9f * t) + tremor;

        // FM vocal synthesis
        float mod = std::sin(2.0f * PI * 55.0f * t) * 60.0f;
        float voice = std::sin(2.0f * PI * (baseFreq + mod) * t);

        // Breathy formant noise
        float breath = (randf() * 2.0f - 1.0f) * 0.18f;
        float sob = (voice + breath);

        // Rhythmic hiccup sobbing envelope
        float hiccup = std::abs(std::sin(2.0f * PI * 1.8f * t));
        hiccup = std::pow(hiccup, 2.5f);

        out[i] = static_cast<int16_t>(clamp(sob * hiccup * 24000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateHollowManShearsSnip() {
    float durationSec = 0.12f;
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float env = std::exp(-45.0f * t);

        // Sharp metallic friction and blade clash
        float clash = std::sin(2.0f * PI * 2850.0f * t) + 0.6f * std::sin(2.0f * PI * 5720.0f * t);
        float friction = (randf() * 2.0f - 1.0f) * 0.4f;

        out[i] = static_cast<int16_t>(clamp((clash + friction) * env * 28000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateHollowManStaticBurst() {
    float durationSec = 0.35f;
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float env = std::exp(-8.0f * t);
        float noise = (randf() * 2.0f - 1.0f);
        // Subsonic thud
        float sub = std::sin(2.0f * PI * 42.0f * t) * 0.6f;

        out[i] = static_cast<int16_t>(clamp((noise * 0.6f + sub) * env * 25000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateChoirChord(bool dissonant, float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    // D3 (146.83 Hz), F3 (174.61 Hz), A3 (220.00 Hz)
    float f1 = 146.83f;
    float f2 = dissonant ? 180.25f : 174.61f; // Dissonant shift
    float f3 = dissonant ? 229.50f : 220.00f;

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);

        // Formant vocal shaping: fundamental + 2nd + 3rd harmonic
        float v1 = std::sin(2.0f * PI * f1 * t) + 0.4f * std::sin(2.0f * PI * f1 * 2.0f * t);
        float v2 = std::sin(2.0f * PI * f2 * t) + 0.4f * std::sin(2.0f * PI * f2 * 2.0f * t);
        float v3 = std::sin(2.0f * PI * f3 * t) + 0.4f * std::sin(2.0f * PI * f3 * 2.0f * t);

        // Throat rasp noise
        float rasp = (randf() * 2.0f - 1.0f) * (dissonant ? 0.22f : 0.08f);

        float s = (v1 + v2 + v3) * 0.33f + rasp;

        float env = 1.0f;
        if (t < 0.6f) env = t / 0.6f;
        else if (t > durationSec - 0.6f) env = (durationSec - t) / 0.6f;

        out[i] = static_cast<int16_t>(clamp(s * env * 22000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateDollGiggle() {
    float durationSec = 1.4f;
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);

        // 5 rapid staccato bursts of pitch-shifted giggles
        float chirpPhase = t * 6.5f;
        float burstEnv = std::sin(2.0f * PI * chirpPhase);
        burstEnv = (burstEnv > 0.0f) ? std::pow(burstEnv, 3.0f) : 0.0f;

        float f = 850.0f + 450.0f * std::sin(2.0f * PI * 14.0f * t);
        float voice = std::sin(2.0f * PI * f * t);

        out[i] = static_cast<int16_t>(clamp(voice * burstEnv * 20000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateWardenKeyJingle() {
    float durationSec = 0.8f;
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    // Multi-metallic clinks
    for (int k = 0; k < 4; ++k) {
        float startTime = k * 0.18f;
        int startSample = static_cast<int>(startTime * SAMPLE_RATE);
        float f1 = 4300.0f + k * 450.0f;
        float f2 = 6800.0f - k * 300.0f;

        for (int i = 0; i < SAMPLE_RATE * 0.15f; ++i) {
            int idx = startSample + i;
            if (idx >= totalSamples) break;

            float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
            float env = std::exp(-35.0f * t);
            float clink = (std::sin(2.0f * PI * f1 * t) + 0.7f * std::sin(2.0f * PI * f2 * t)) * env;

            float cur = static_cast<float>(out[idx]);
            out[idx] = static_cast<int16_t>(clamp(cur + clink * 14000.0f, -32767.0f, 32767.0f));
        }
    }
    return out;
}

std::vector<int16_t> Synth2::generateWardenChainDrag(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        // Low iron resonance + metallic drag noise
        float ironRes = std::sin(2.0f * PI * 340.0f * t) * 0.35f + std::sin(2.0f * PI * 820.0f * t) * 0.25f;
        float metalRattle = (randf() * 2.0f - 1.0f) * 0.5f;

        // Periodic chain link thumps
        float linkThump = std::abs(std::sin(2.0f * PI * 3.2f * t));
        linkThump = std::pow(linkThump, 4.0f);

        float s = (ironRes + metalRattle) * (0.4f + 0.6f * linkThump);
        out[i] = static_cast<int16_t>(clamp(s * 22000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::generateWardenGutturalRoar(float durationSec) {
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float progress = t / durationSec;

        // Pitch sweeps 55 Hz -> 42 Hz
        float f = 55.0f - 13.0f * progress;
        float sub = std::sin(2.0f * PI * f * t);
        // Heavy distortion saturation
        float dist = std::tanh(sub * 3.5f);

        // Growl noise
        float growl = (randf() * 2.0f - 1.0f) * 0.3f;
        float env = std::sin(PI * progress);

        out[i] = static_cast<int16_t>(clamp((dist + growl) * env * 26000.0f, -32767.0f, 32767.0f));
    }
    return out;
}

std::vector<int16_t> Synth2::applyTapeDegradation(const std::vector<int16_t>& input) {
    std::vector<int16_t> output(input.size());
    float lpf = 0.0f;
    float hpf = 0.0f;

    for (size_t i = 0; i < input.size(); ++i) {
        float in = static_cast<float>(input[i]);

        // Bandpass 300 Hz to 3400 Hz (vintage tape/radio telephone sound)
        lpf += 0.22f * (in - lpf);
        hpf += 0.04f * (lpf - hpf);
        float bandpassed = lpf - hpf;

        // Vintage tape hiss
        float hiss = (randf() * 2.0f - 1.0f) * 650.0f;

        // Mild magnetic tape saturation
        float saturated = std::tanh(bandpassed / 20000.0f) * 22000.0f;

        output[i] = static_cast<int16_t>(clamp(saturated + hiss, -32767.0f, 32767.0f));
    }
    return output;
}

} // namespace hm
