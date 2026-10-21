#include <cassert>
#include <iostream>
#include "world/NavGrid.h"
#include "ai/Pathfind.h"

using namespace hm;

int main() {
    std::cout << "[TEST] Running Pathfind A* test suite..." << std::endl;

    NavGrid nav;
    nav.init(20, 20, 1.0f, Vec3(0, 0, 0));

    // Place a wall obstacle across x=10 from z=5 to z=15 with a gap at z=12
    for (int z = 5; z <= 15; ++z) {
        if (z != 12) {
            nav.setWalkable(10, z, false);
        }
    }

    Vec3 start(2.5f, 0.0f, 10.5f);
    Vec3 goal(16.5f, 0.0f, 10.5f);

    auto path = Pathfind::findPath(nav, start, goal, true);
    assert(!path.empty());
    std::cout << "  ✓ A* found path around obstacle (waypoints: " << path.size() << ")" << std::endl;

    // Verify destination is reached
    Vec3 endPos = path.back();
    float distToGoal = distance(endPos, goal);
    assert(distToGoal < 1.5f);
    std::cout << "  ✓ Goal proximity verified (dist: " << distToGoal << ")" << std::endl;

    std::cout << "[TEST] ALL PATHFINDING TESTS PASSED!" << std::endl;
    return 0;
}
