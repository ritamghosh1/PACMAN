#include "SafeRoom.h"

namespace hm {

SafeRoomManager::SafeRoomManager() {
    m_rooms = {
        {0, "Foyer Antechamber", Vec3(2.0f, 0.0f, 2.0f), 3.5f, true},
        {1, "Library Stacks Office", Vec3(10.0f, 0.0f, 6.0f), 3.0f, false},
        {2, "Potting Shed Sanctuary", Vec3(-8.0f, 0.0f, 6.0f), 3.0f, false},
        {3, "Boiler Supervisor Booth", Vec3(8.0f, 0.0f, -8.0f), 3.0f, false},
        {4, "Nursery Sewing Nook", Vec3(-6.0f, 0.0f, -12.0f), 3.0f, false},
        {5, "Vestry Confessional", Vec3(14.0f, 0.0f, 12.0f), 3.5f, false},
        {6, "East Wing Dispensary", Vec3(4.0f, 0.0f, 14.0f), 2.8f, false},
        {7, "Matron's Quarters", Vec3(-12.0f, 0.0f, 2.0f), 3.0f, false}
    };
}

bool SafeRoomManager::isInSafeRoom(const Vec3& playerPos) const {
    for (const auto& r : m_rooms) {
        if (distance(playerPos, r.center) <= r.radius) {
            return true;
        }
    }
    return false;
}

} // namespace hm
