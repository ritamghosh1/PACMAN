#pragma once
#include <cstdint>

#include <vector>
#include "core/mathx.h"
#include "world/Collision.h"

namespace hm {

class NavGrid {
public:
    NavGrid() = default;

    void init(int width, int depth, float cellSize, const Vec3& origin);
    void buildFromCollision(const Collision& collision, float ghostRadius = 0.35f);

    bool isWalkable(int gx, int gz) const;
    void setWalkable(int gx, int gz, bool walkable);

    bool worldToGrid(const Vec3& worldPos, int& gx, int& gz) const;
    Vec3 gridToWorld(int gx, int gz) const;

    int width() const { return m_width; }
    int depth() const { return m_depth; }
    float cellSize() const { return m_cellSize; }
    const Vec3& origin() const { return m_origin; }

    float getCellCost(int gx, int gz) const;
    void setCellCost(int gx, int gz, float cost);

private:
    int m_width = 0;
    int m_depth = 0;
    float m_cellSize = 0.5f;
    Vec3 m_origin{0, 0, 0};
    std::vector<uint8_t> m_walkable;
    std::vector<float> m_costs;
};

} // namespace hm
