#include "ValvePuzzle.h"

namespace hm {

ValvePuzzle::ValvePuzzle() {
    reset();
}

void ValvePuzzle::reset() {
    for (int i = 0; i < 4; ++i) {
        m_valveState[i] = false;
    }
    m_currentPressure = 0;
    m_solved = false;
}

void ValvePuzzle::update(float /*dt*/) {
}

void ValvePuzzle::toggleValve(int valveIndex) {
    if (valveIndex >= 0 && valveIndex < 4) {
        m_valveState[valveIndex] = !m_valveState[valveIndex];
        calculatePressure();
    }
}

bool ValvePuzzle::isValveOpen(int valveIndex) const {
    if (valveIndex >= 0 && valveIndex < 4) {
        return m_valveState[valveIndex];
    }
    return false;
}

void ValvePuzzle::calculatePressure() {
    m_currentPressure = 0;
    for (int i = 0; i < 4; ++i) {
        if (m_valveState[i]) {
            m_currentPressure += m_valveDeltas[i];
        }
    }
    m_solved = (m_currentPressure == 140);
}

} // namespace hm
