#pragma once

#include "Fdn.h"
#include "EarlyRefl.h"
#include <string>

namespace hm {

enum class WingAcousticType {
    StoneHall,     // Foyer: vast stone, moderate damping, 2.2s RT60
    WoodLibrary,   // Library: book absorption, warm, 1.2s RT60
    GlassDome,     // Conservatory: bright reflections, 1.7s RT60
    DampCellar,    // Boiler Cellar: wet stone slap, low damping, 1.9s RT60
    SoftNursery,   // Nursery: fabric/carpet, high absorption, 0.8s RT60
    Cathedral      // Chapel: massive space, long 3.5s RT60
};

class ReverbManager {
public:
    ReverbManager();

    void setWing(WingAcousticType type);
    WingAcousticType currentWing() const { return m_type; }

    void processStereo(std::vector<int16_t>& stereoSamples, float wetFactor = 1.0f);

private:
    WingAcousticType m_type = WingAcousticType::StoneHall;
    FdnReverb m_fdn;
    EarlyReflections m_earlyRefl;
};

} // namespace hm
