#include "SafeRoom.h"

namespace hm {

const SafeRoomZone* SafeRoomManager::zoneAt(const Vec3& p) const {
    for (const auto& r : m_rooms) {
        float dx = p.x - r.center.x, dz = p.z - r.center.z;
        if (dx * dx + dz * dz <= r.radius * r.radius) return &r;
    }
    return nullptr;
}

bool SafeRoomManager::isInSafeRoom(const Vec3& playerPos) const {
    return zoneAt(playerPos) != nullptr;
}

} // namespace hm
