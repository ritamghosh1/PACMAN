#include "motifs.h"
#include "audio/Synth2.h"
#include "core/mathx.h"
#include <cmath>

namespace hm {
namespace scores {

std::vector<int16_t> renderHushLittleLight(float tempoBpm) {
    // 8-bar melody in D minor:
    // D4 (293.66), F4 (349.23), A4 (440.00), G4 (392.00)
    // F4 (349.23), E4 (329.63), D4 (293.66), C#4 (277.18), D4 (293.66)
    const float notes[] = {293.66f, 349.23f, 440.00f, 392.00f, 349.23f, 329.63f, 293.66f, 277.18f, 293.66f};
    float beatSec = 60.0f / tempoBpm;

    int totalSamples = static_cast<int>(Synth2::SAMPLE_RATE * (9 * beatSec + 2.5f));
    std::vector<float> mixBuffer(totalSamples, 0.0f);

    for (int n = 0; n < 9; ++n) {
        auto bell = Synth2::generateMusicBoxNote(notes[n], 2.2f);
        int start = static_cast<int>(n * beatSec * Synth2::SAMPLE_RATE);
        for (size_t i = 0; i < bell.size() && start + i < mixBuffer.size(); ++i) {
            mixBuffer[start + i] += static_cast<float>(bell[i]);
        }
    }

    std::vector<int16_t> out(totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        out[i] = static_cast<int16_t>(clamp(mixBuffer[i] * 0.8f, -32767.0f, 32767.0f));
    }
    return out;
}

} // namespace scores
} // namespace hm
