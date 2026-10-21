#include "CipherPuzzle.h"

namespace hm {

CipherPuzzle::CipherPuzzle() {
    reset();
}

void CipherPuzzle::reset() {
    m_enteredSequence.clear();
    m_solved = false;
}

void CipherPuzzle::update(float /*dt*/) {
}

bool CipherPuzzle::pushBook(int bookIndex) {
    if (m_solved) return true;

    // Correct target sequence: 0, 1, 2, 3
    int expected = static_cast<int>(m_enteredSequence.size());
    if (bookIndex == expected) {
        m_enteredSequence.push_back(bookIndex);
        if (m_enteredSequence.size() == 4) {
            m_solved = true;
        }
        return true;
    } else {
        // Wrong sequence, reset with mechanical click
        m_enteredSequence.clear();
        return false;
    }
}

} // namespace hm
