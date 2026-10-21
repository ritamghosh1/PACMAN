#pragma once

#include <vector>
#include "core/mathx.h"
#include "world/NavGrid.h"

namespace hm {

class Pathfind {
public:
    static std::vector<Vec3> findPath(const NavGrid& nav,
                                      const Vec3& startPos,
                                      const Vec3& goalPos,
                                      bool smooth = true);

private:
    static std::vector<Vec3> smoothPath(const NavGrid& nav, const std::vector<Vec3>& rawPath);
    static bool lineOfSight(const NavGrid& nav, const Vec3& p1, const Vec3& p2);
};

} // namespace hm
