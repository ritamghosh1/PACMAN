#include "MirrorPuzzle.h"

namespace hm {

MirrorPuzzle::MirrorPuzzle() {
    reset();
}

void MirrorPuzzle::reset() {
    m_rotations[0] = 0;
    m_rotations[1] = 0;
    m_rotations[2] = 0;
    m_solved = false;
}

void MirrorPuzzle::update(float /*dt*/) {
}

void MirrorPuzzle::rotateMirror(int mirrorIndex) {
    if (mirrorIndex >= 0 && mirrorIndex < 3) {
        m_rotations[mirrorIndex] = (m_rotations[mirrorIndex] + 1) % 4;
        checkAlignment();
    }
}

int MirrorPuzzle::mirrorOrientation(int mirrorIndex) const {
    if (mirrorIndex >= 0 && mirrorIndex < 3) {
        return m_rotations[mirrorIndex];
    }
    return 0;
}

bool MirrorPuzzle::checkAlignment() {
    // Solved when: Mirror 0 = 1 (90°), Mirror 1 = 2 (180°), Mirror 2 = 3 (270°)
    if (m_rotations[0] == 1 && m_rotations[1] == 2 && m_rotations[2] == 3) {
        m_solved = true;
        return true;
    }
    m_solved = false;
    return false;
}

} // namespace hm
