#pragma once

#include "core/mathx.h"
#include "audio/Reverb.h"
#include "audio/MusicDirector.h"
#include <string>

namespace hm {

struct WingDefinition {
    int wingId = 0;
    std::string name;
    WingAcousticType acousticType = WingAcousticType::StoneHall;
    MusicState musicState = MusicState::DreadBed;
    Vec3 ambientLight{0.08f, 0.08f, 0.12f};
    Vec3 fogColor{0.04f, 0.04f, 0.06f};
    float fogDensity = 0.045f;
    bool hasSafeRoom = true;
    Vec3 safeRoomCenter{0, 0, 0};
};

class WingsManager {
public:
    static WingDefinition getWing(int id);
};

} // namespace hm
