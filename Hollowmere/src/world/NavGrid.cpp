#include "NavGrid.h"
#include <cmath>

namespace hm {

void NavGrid::init(int width, int depth, float cellSize, const Vec3& origin) {
    m_width = width;
    m_depth = depth;
    m_cellSize = cellSize;
    m_origin = origin;
    m_walkable.assign(m_width * m_depth, 1);
    m_costs.assign(m_width * m_depth, 1.0f);
}

void NavGrid::buildFromCollision(const Collision& collision, float ghostRadius) {
    for (int gz = 0; gz < m_depth; ++gz) {
        for (int gx = 0; gx < m_width; ++gx) {
            Vec3 worldCenter = gridToWorld(gx, gz);
            AABB sampleBox;
            sampleBox.min = worldCenter - Vec3(ghostRadius, 0.0f, ghostRadius);
            sampleBox.max = worldCenter + Vec3(ghostRadius, 2.0f, ghostRadius);

            bool blocked = false;
            for (const auto& col : collision.colliders()) {
                if (col.enabled && col.tag == 0 && sampleBox.intersects(col.box)) {
                    blocked = true;
                    break;
                }
            }

            setWalkable(gx, gz, !blocked);
        }
    }
}

bool NavGrid::isWalkable(int gx, int gz) const {
    if (gx < 0 || gx >= m_width || gz < 0 || gz >= m_depth) return false;
    return m_walkable[gz * m_width + gx] != 0;
}

void NavGrid::setWalkable(int gx, int gz, bool walkable) {
    if (gx >= 0 && gx < m_width && gz >= 0 && gz < m_depth) {
        m_walkable[gz * m_width + gx] = walkable ? 1 : 0;
    }
}

bool NavGrid::worldToGrid(const Vec3& worldPos, int& gx, int& gz) const {
    gx = static_cast<int>(std::floor((worldPos.x - m_origin.x) / m_cellSize));
    gz = static_cast<int>(std::floor((worldPos.z - m_origin.z) / m_cellSize));
    return (gx >= 0 && gx < m_width && gz >= 0 && gz < m_depth);
}

Vec3 NavGrid::gridToWorld(int gx, int gz) const {
    return Vec3(
        m_origin.x + (static_cast<float>(gx) + 0.5f) * m_cellSize,
        m_origin.y,
        m_origin.z + (static_cast<float>(gz) + 0.5f) * m_cellSize
    );
}

float NavGrid::getCellCost(int gx, int gz) const {
    if (gx < 0 || gx >= m_width || gz < 0 || gz >= m_depth) return 999.0f;
    return m_costs[gz * m_width + gx];
}

void NavGrid::setCellCost(int gx, int gz, float cost) {
    if (gx >= 0 && gx < m_width && gz >= 0 && gz < m_depth) {
        m_costs[gz * m_width + gx] = cost;
    }
}

} // namespace hm
