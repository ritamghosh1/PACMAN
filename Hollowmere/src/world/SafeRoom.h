#pragma once

#include "core/mathx.h"
#include <vector>
#include <string>

namespace hm {

struct SafeRoomZone {
    int id = 0;
    std::string name;
    Vec3 center;
    float radius = 3.5f;
    bool discovered = false;
};

class SafeRoomManager {
public:
    SafeRoomManager();

    bool isInSafeRoom(const Vec3& playerPos) const;
    const std::vector<SafeRoomZone>& rooms() const { return m_rooms; }

private:
    std::vector<SafeRoomZone> m_rooms;
};

} // namespace hm
