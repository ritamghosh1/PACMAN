#pragma once

#include "Puzzle.h"
#include <vector>

namespace hm {

class SigilAltar : public IPuzzle {
public:
    SigilAltar();

    const std::string& name() const override { return m_name; }
    bool isSolved() const override { return m_placedCount >= 5; }
    void reset() override;
    void update(float dt) override;
    int targetUnlockDoorId() const override { return 500; } // Chapel Crypt Gate

    bool placeSigil(int sigilId);
    int placedCount() const { return m_placedCount; }
    bool isSigilPlaced(int sigilId) const;

private:
    std::string m_name = "Sigil Altar";
    bool m_sigils[5] = {false, false, false, false, false};
    int m_placedCount = 0;
};

} // namespace hm
