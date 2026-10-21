#pragma once

#include "core/mathx.h"
#include <vector>

namespace hm {

struct Portal {
    Vec3 position;      // Center of door or archway opening
    bool isClosed = false; // Closed door = -18dB and 900Hz LPF
    float width = 1.2f;
};

struct PropagatedSound {
    Vec3 apparentPosition; // Where the listener perceives the sound coming from
    float totalDistance;   // Total acoustic distance along portals
    bool isDirectPath;     // True if clear line-of-sight
    bool isBlockedByDoor;  // True if passes through a closed door
    float extraAttenDb;    // Attenuation in dB from wall/door penetration
    float cutoffHz;        // Occlusion lowpass cutoff
};

class SoundPropagation {
public:
    static PropagatedSound resolve(const Vec3& listenerPos,
                                   const Vec3& emitterPos,
                                   const std::vector<Portal>& portals,
                                   bool hasDirectLineOfSight = true);
};

} // namespace hm
