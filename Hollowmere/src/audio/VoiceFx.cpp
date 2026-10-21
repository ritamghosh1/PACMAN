#include "VoiceFx.h"
#include "core/mathx.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace hm {

static inline float randf() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

std::vector<int16_t> VoiceFx::synthesizePhrase(const std::string& lineText, VoiceCharacter character) {
    // Generate duration proportional to text length
    float words = std::max(1.0f, static_cast<float>(lineText.length()) / 5.0f);
    float durationSec = std::clamp(words * 0.35f, 1.2f, 7.5f);
    int totalSamples = static_cast<int>(SAMPLE_RATE * durationSec);
    std::vector<int16_t> out(totalSamples);

    float basePitch = 140.0f;
    float formantF1 = 500.0f;
    float formantF2 = 1500.0f;
    bool isWhisper = false;

    switch (character) {
        case VoiceCharacter::Mae:
            basePitch = 290.0f;
            formantF1 = 650.0f;
            formantF2 = 2200.0f;
            break;
        case VoiceCharacter::DrCrane:
            basePitch = 105.0f; // Deep, slow, cold
            formantF1 = 450.0f;
            formantF2 = 1200.0f;
            break;
        case VoiceCharacter::Nora:
            basePitch = 210.0f;
            formantF1 = 550.0f;
            formantF2 = 1800.0f;
            break;
        case VoiceCharacter::WhisperingChild:
            basePitch = 250.0f;
            isWhisper = true;
            formantF1 = 700.0f;
            formantF2 = 2400.0f;
            break;
    }

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);

        // Intonation curve with gentle speech cadence
        float speechCadence = std::sin(2.0f * PI * 3.2f * t);
        float pitch = basePitch + speechCadence * 12.0f;

        // Glottal pulse or whisper noise
        float source = 0.0f;
        if (isWhisper) {
            source = (randf() * 2.0f - 1.0f);
        } else {
            float phase = std::fmod(t * pitch, 1.0f);
            source = (phase < 0.2f) ? std::sin(PI * (phase / 0.2f)) : -0.15f;
        }

        // Formant resonance filters
        float f1 = std::sin(2.0f * PI * formantF1 * t) * 0.6f;
        float f2 = std::sin(2.0f * PI * formantF2 * t) * 0.4f;

        float voice = source * (f1 + f2);

        // Syllable pulsing envelope
        float syllable = std::abs(std::sin(2.0f * PI * 4.2f * t));
        syllable = std::pow(syllable, 1.8f);

        // Overall speech envelope
        float overallEnv = 1.0f;
        if (t < 0.15f) overallEnv = t / 0.15f;
        else if (t > durationSec - 0.25f) overallEnv = (durationSec - t) / 0.25f;

        float s = voice * syllable * overallEnv;
        out[i] = static_cast<int16_t>(clamp(s * 22000.0f, -32767.0f, 32767.0f));
    }

    if (character == VoiceCharacter::DrCrane) {
        return processCraneTape(out);
    } else if (character == VoiceCharacter::Mae || character == VoiceCharacter::WhisperingChild) {
        return applyReverseSwell(out);
    }

    return out;
}

std::vector<int16_t> VoiceFx::processCraneTape(const std::vector<int16_t>& dryVoice) {
    std::vector<int16_t> out(dryVoice.size());
    float lpf = 0.0f;
    float hpf = 0.0f;

    for (size_t i = 0; i < dryVoice.size(); ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
        float in = static_cast<float>(dryVoice[i]);

        // Bandpass 350 Hz to 2800 Hz
        lpf += 0.20f * (in - lpf);
        hpf += 0.045f * (lpf - hpf);
        float filtered = lpf - hpf;

        // Wow and flutter subtle tape pitch warble
        float flutter = std::sin(2.0f * PI * 1.8f * t) * 0.08f;
        filtered *= (1.0f + flutter);

        // Tape saturation distortion
        float sat = std::tanh(filtered / 16000.0f) * 18000.0f;

        // Continuous magnetic tape hiss
        float hiss = (randf() * 2.0f - 1.0f) * 750.0f;

        out[i] = static_cast<int16_t>(clamp(sat + hiss, -32767.0f, 32767.0f));
    }

    return out;
}

std::vector<int16_t> VoiceFx::applyReverseSwell(const std::vector<int16_t>& dryVoice) {
    if (dryVoice.empty()) return dryVoice;

    std::vector<int16_t> reversed = dryVoice;
    std::reverse(reversed.begin(), reversed.end());

    // Simple forward echo on reversed audio
    std::vector<float> echo(reversed.size(), 0.0f);
    int delay = static_cast<int>(SAMPLE_RATE * 0.22f);

    for (size_t i = 0; i < reversed.size(); ++i) {
        echo[i] += static_cast<float>(reversed[i]);
        if (i >= static_cast<size_t>(delay)) {
            echo[i] += static_cast<float>(reversed[i - delay]) * 0.45f;
        }
    }

    // Reverse back to get reverse swell leading into voice
    std::vector<int16_t> out(echo.size());
    for (size_t i = 0; i < echo.size(); ++i) {
        size_t revIdx = echo.size() - 1 - i;
        out[i] = static_cast<int16_t>(clamp(echo[revIdx], -32767.0f, 32767.0f));
    }

    return out;
}

} // namespace hm
