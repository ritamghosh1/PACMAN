#include "Propagation.h"
#include <limits>
#include <algorithm>

namespace hm {

PropagatedSound SoundPropagation::resolve(const Vec3& listenerPos,
                                         const Vec3& emitterPos,
                                         const std::vector<Portal>& portals,
                                         bool hasDirectLineOfSight) {
    PropagatedSound result;

    if (hasDirectLineOfSight || portals.empty()) {
        result.apparentPosition = emitterPos;
        result.totalDistance = distance(listenerPos, emitterPos);
        result.isDirectPath = hasDirectLineOfSight;
        result.isBlockedByDoor = false;
        result.extraAttenDb = hasDirectLineOfSight ? 0.0f : -9.0f;
        result.cutoffHz = hasDirectLineOfSight ? 20000.0f : 1800.0f;
        return result;
    }

    // Find the best portal that minimizes path: listener -> portal -> emitter
    float bestPathLen = std::numeric_limits<float>::max();
    int bestIdx = -1;

    for (size_t i = 0; i < portals.size(); ++i) {
        float d1 = distance(listenerPos, portals[i].position);
        float d2 = distance(portals[i].position, emitterPos);
        float total = d1 + d2;
        if (total < bestPathLen) {
            bestPathLen = total;
            bestIdx = static_cast<int>(i);
        }
    }

    if (bestIdx >= 0) {
        const Portal& p = portals[bestIdx];
        result.apparentPosition = p.position;
        result.totalDistance = bestPathLen;
        result.isDirectPath = false;
        result.isBlockedByDoor = p.isClosed;

        if (p.isClosed) {
            result.extraAttenDb = -18.0f; // Door shut: -18dB
            result.cutoffHz = 900.0f;     // Severe high-frequency damping
        } else {
            result.extraAttenDb = -4.5f;  // Door open: minor diffraction loss
            result.cutoffHz = 4200.0f;
        }
    } else {
        result.apparentPosition = emitterPos;
        result.totalDistance = distance(listenerPos, emitterPos);
        result.isDirectPath = false;
        result.isBlockedByDoor = false;
        result.extraAttenDb = -12.0f;
        result.cutoffHz = 1200.0f;
    }

    return result;
}

} // namespace hm
