#pragma once

#include "Puzzle.h"

namespace hm {

class ValvePuzzle : public IPuzzle {
public:
    ValvePuzzle();

    const std::string& name() const override { return m_name; }
    bool isSolved() const override { return m_solved; }
    void reset() override;
    void update(float dt) override;
    int targetUnlockDoorId() const override { return 301; } // Boiler Drainage Gate

    void toggleValve(int valveIndex);
    bool isValveOpen(int valveIndex) const;
    int currentPressure() const { return m_currentPressure; }

private:
    void calculatePressure();

    std::string m_name = "Boiler Steam Valves";
    bool m_valveState[4] = {false, false, false, false};
    const int m_valveDeltas[4] = {40, 60, -20, 80};
    int m_currentPressure = 0;
    bool m_solved = false;
};

} // namespace hm
