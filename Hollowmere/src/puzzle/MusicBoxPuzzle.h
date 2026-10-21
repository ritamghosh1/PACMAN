#pragma once

#include "Puzzle.h"
#include <vector>

namespace hm {

class MusicBoxPuzzle : public IPuzzle {
public:
    MusicBoxPuzzle();

    const std::string& name() const override { return m_name; }
    bool isSolved() const override { return m_solved; }
    void reset() override;
    void update(float dt) override;
    int targetUnlockDoorId() const override { return 401; } // Nursery Toy Chest

    // Player strikes bell index 0..3
    bool strikeBell(int bellIndex);
    int currentStep() const { return static_cast<int>(m_playedMelody.size()); }

private:
    std::string m_name = "Nursery Music Box Melody";
    std::vector<int> m_playedMelody;
    bool m_solved = false;
};

} // namespace hm
