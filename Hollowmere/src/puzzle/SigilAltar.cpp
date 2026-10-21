#include "SigilAltar.h"

namespace hm {

SigilAltar::SigilAltar() {
    reset();
}

void SigilAltar::reset() {
    for (int i = 0; i < 5; ++i) {
        m_sigils[i] = false;
    }
    m_placedCount = 0;
}

void SigilAltar::update(float /*dt*/) {
}

bool SigilAltar::placeSigil(int sigilId) {
    if (sigilId < 0 || sigilId >= 5) return false;
    if (!m_sigils[sigilId]) {
        m_sigils[sigilId] = true;
        m_placedCount++;
        return true;
    }
    return false;
}

bool SigilAltar::isSigilPlaced(int sigilId) const {
    if (sigilId < 0 || sigilId >= 5) return false;
    return m_sigils[sigilId];
}

} // namespace hm
