#pragma once

#include "Puzzle.h"

namespace hm {

class MirrorPuzzle : public IPuzzle {
public:
    MirrorPuzzle();

    const std::string& name() const override { return m_name; }
    bool isSolved() const override { return m_solved; }
    void reset() override;
    void update(float dt) override;
    int targetUnlockDoorId() const override { return 201; } // Conservatory Gate

    // Rotate mirror (0, 1, or 2) by 90 degrees clockwise
    void rotateMirror(int mirrorIndex);
    int mirrorOrientation(int mirrorIndex) const;

    bool checkAlignment();

private:
    std::string m_name = "Conservatory Mirror Reflection";
    int m_rotations[3] = {0, 0, 0}; // 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
    bool m_solved = false;
};

} // namespace hm
