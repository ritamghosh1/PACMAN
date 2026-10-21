#pragma once

#include <vector>
#include <cstdint>

namespace hm {
namespace scores {

// Renders the canonical "Hush Little Light" 8-bar leitmotif
std::vector<int16_t> renderHushLittleLight(float tempoBpm = 68.0f);

// Full intro cutscene soundtrack (75-90 seconds)
std::vector<int16_t> renderIntroScore();

// Ending scores for E1 through E5
std::vector<int16_t> renderEndingScoreE1(); // Release & Light (resolves to D Major)
std::vector<int16_t> renderEndingScoreE2(); // Consumed / Shadows
std::vector<int16_t> renderEndingScoreE3(); // Trapped in the Loop
std::vector<int16_t> renderEndingScoreE4(); // Mercy Harmonization
std::vector<int16_t> renderEndingScoreE5(); // The Warden's Truth

inline std::vector<int16_t> renderEndingScore(int type) {
    switch (type) {
        case 0: return renderEndingScoreE1();
        case 1: return renderEndingScoreE2();
        case 2: return renderEndingScoreE3();
        case 3: return renderEndingScoreE4();
        default: return renderEndingScoreE5();
    }
}

} // namespace scores
} // namespace hm
