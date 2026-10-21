#include "MusicBoxPuzzle.h"

namespace hm {

MusicBoxPuzzle::MusicBoxPuzzle() {
    reset();
}

void MusicBoxPuzzle::reset() {
    m_playedMelody.clear();
    m_solved = false;
}

void MusicBoxPuzzle::update(float /*dt*/) {
}

bool MusicBoxPuzzle::strikeBell(int bellIndex) {
    if (m_solved) return true;

    // Target leitmotif sequence: D4 (0), F4 (1), A4 (2), D4 (0)
    const int targetSequence[4] = {0, 1, 2, 0};
    int step = static_cast<int>(m_playedMelody.size());

    if (bellIndex == targetSequence[step]) {
        m_playedMelody.push_back(bellIndex);
        if (m_playedMelody.size() == 4) {
            m_solved = true;
        }
        return true;
    } else {
        // Wrong note: dissonant reset
        m_playedMelody.clear();
        return false;
    }
}

} // namespace hm
