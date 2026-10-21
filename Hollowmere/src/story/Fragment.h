#pragma once

#include "core/mathx.h"
#include <string>
#include <vector>

namespace hm {

struct MemoryFragment {
    int id = 0;
    std::string title;
    std::string location;
    std::string narrativeText;
    Vec3 triggerPos;
    float triggerRadius = 1.5f;
    bool discovered = false;
};

class MemorySystem {
public:
    MemorySystem();

    void initDefaultFragments();
    bool checkTrigger(const Vec3& playerPos, MemoryFragment& outFragment);

    int totalDiscovered() const { return m_discoveredCount; }
    const std::vector<MemoryFragment>& fragments() const { return m_fragments; }

private:
    std::vector<MemoryFragment> m_fragments;
    int m_discoveredCount = 0;
};

} // namespace hm
