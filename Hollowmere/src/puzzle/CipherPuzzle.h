#pragma once

#include "Puzzle.h"
#include <vector>

namespace hm {

class CipherPuzzle : public IPuzzle {
public:
    CipherPuzzle();

    const std::string& name() const override { return m_name; }
    bool isSolved() const override { return m_solved; }
    void reset() override;
    void update(float dt) override;
    int targetUnlockDoorId() const override { return 101; } // Library Archive Door

    // Player interacts with book index 0..3
    bool pushBook(int bookIndex);
    int currentStep() const { return static_cast<int>(m_enteredSequence.size()); }

private:
    std::string m_name = "Library Book Cipher";
    std::vector<int> m_enteredSequence;
    bool m_solved = false;
};

} // namespace hm
