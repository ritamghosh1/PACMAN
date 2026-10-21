#pragma once

#include <vector>
#include <cstdint>

namespace hm {

class Synth2 {
public:
    static constexpr int SAMPLE_RATE = 44100;

    // Music & melodic synthesis
    static std::vector<int16_t> generateMusicBoxNote(float freqHz, float durationSec = 2.0f);
    static std::vector<int16_t> generateMusicBoxMotif(); // "Hush Little Light" leitmotif
    static std::vector<int16_t> generateFeltPianoNote(float freqHz, float durationSec = 2.5f);
    static std::vector<int16_t> generateBowedCelloDrone(float baseFreqHz = 73.42f, float durationSec = 4.0f); // D2
    static std::vector<int16_t> generateDissonantStringRiser(float durationSec = 3.5f);
    static std::vector<int16_t> generateSubBassHit(float durationSec = 2.0f);
    static std::vector<int16_t> generateGlassHarmonicaChord(float durationSec = 3.0f);

    // Ghost acoustic signatures
    static std::vector<int16_t> generateWeeperSobbing(float durationSec = 3.0f);
    static std::vector<int16_t> generateHollowManShearsSnip();
    static std::vector<int16_t> generateHollowManStaticBurst();
    static std::vector<int16_t> generateChoirChord(bool dissonant = false, float durationSec = 4.0f);
    static std::vector<int16_t> generateDollGiggle();
    static std::vector<int16_t> generateWardenKeyJingle();
    static std::vector<int16_t> generateWardenChainDrag(float durationSec = 2.0f);
    static std::vector<int16_t> generateWardenGutturalRoar(float durationSec = 2.5f);

    // Audio tape log processing & tape hiss/wow-flutter
    static std::vector<int16_t> applyTapeDegradation(const std::vector<int16_t>& input);
};

} // namespace hm
