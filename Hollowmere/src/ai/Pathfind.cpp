#include "Pathfind.h"
#include <queue>
#include <cmath>
#include <algorithm>

namespace hm {

struct Node {
    int x = 0;
    int z = 0;
    float g = 0.0f;
    float f = 0.0f;
    int px = -1;
    int pz = -1;
    bool closed = false;
};

struct CompareNode {
    bool operator()(const Node* a, const Node* b) const {
        return a->f > b->f;
    }
};

static inline float heuristic(int x1, int z1, int x2, int z2) {
    float dx = std::abs(static_cast<float>(x1 - x2));
    float dz = std::abs(static_cast<float>(z1 - z2));
    // Octile distance
    return (dx + dz) + (1.41421356f - 2.0f) * std::min(dx, dz);
}

std::vector<Vec3> Pathfind::findPath(const NavGrid& nav,
                                    const Vec3& startPos,
                                    const Vec3& goalPos,
                                    bool smooth) {
    std::vector<Vec3> path;
    int sx, sz, gx, gz;
    if (!nav.worldToGrid(startPos, sx, sz) || !nav.worldToGrid(goalPos, gx, gz)) {
        return path;
    }

    if (!nav.isWalkable(gx, gz)) {
        // Find nearest walkable neighbor to goal
        bool found = false;
        for (int r = 1; r <= 3 && !found; ++r) {
            for (int dz = -r; dz <= r && !found; ++dz) {
                for (int dx = -r; dx <= r && !found; ++dx) {
                    if (nav.isWalkable(gx + dx, gz + dz)) {
                        gx += dx;
                        gz += dz;
                        found = true;
                    }
                }
            }
        }
        if (!found) return path;
    }

    int width = nav.width();
    int depth = nav.depth();
    std::vector<Node> grid(width * depth);
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            int idx = z * width + x;
            grid[idx].x = x;
            grid[idx].z = z;
            grid[idx].g = 1e9f;
            grid[idx].f = 1e9f;
            grid[idx].px = -1;
            grid[idx].pz = -1;
            grid[idx].closed = false;
        }
    }

    std::priority_queue<Node*, std::vector<Node*>, CompareNode> openSet;

    int startIdx = sz * width + sx;
    grid[startIdx].g = 0.0f;
    grid[startIdx].f = heuristic(sx, sz, gx, gz);
    openSet.push(&grid[startIdx]);

    bool arrived = false;
    const int dxs[8] = {1, -1, 0, 0, 1, -1, 1, -1};
    const int dzs[8] = {0, 0, 1, -1, 1, 1, -1, -1};
    const float stepCosts[8] = {1.0f, 1.0f, 1.0f, 1.0f, 1.414f, 1.414f, 1.414f, 1.414f};

    int iterations = 0;
    const int maxIterations = 2500;

    while (!openSet.empty() && iterations++ < maxIterations) {
        Node* current = openSet.top();
        openSet.pop();

        if (current->closed) continue;
        current->closed = true;

        if (current->x == gx && current->z == gz) {
            arrived = true;
            break;
        }

        for (int i = 0; i < 8; ++i) {
            int nx = current->x + dxs[i];
            int nz = current->z + dzs[i];

            if (nx < 0 || nx >= width || nz < 0 || nz >= depth) continue;
            if (!nav.isWalkable(nx, nz)) continue;

            // Diagonal corner-cutting check
            if (i >= 4) {
                if (!nav.isWalkable(current->x + dxs[i], current->z) ||
                    !nav.isWalkable(current->x, current->z + dzs[i])) {
                    continue;
                }
            }

            int nIdx = nz * width + nx;
            if (grid[nIdx].closed) continue;

            float cellWeight = nav.getCellCost(nx, nz);
            float tentativeG = current->g + stepCosts[i] * cellWeight;

            if (tentativeG < grid[nIdx].g) {
                grid[nIdx].px = current->x;
                grid[nIdx].pz = current->z;
                grid[nIdx].g = tentativeG;
                grid[nIdx].f = tentativeG + heuristic(nx, nz, gx, gz);
                openSet.push(&grid[nIdx]);
            }
        }
    }

    if (!arrived) return path;

    // Trace path back
    int curX = gx;
    int curZ = gz;
    while (curX != -1 && curZ != -1) {
        path.push_back(nav.gridToWorld(curX, curZ));
        if (curX == sx && curZ == sz) break;
        int idx = curZ * width + curX;
        curX = grid[idx].px;
        curZ = grid[idx].pz;
    }

    std::reverse(path.begin(), path.end());

    if (smooth && path.size() > 2) {
        return smoothPath(nav, path);
    }

    return path;
}

bool Pathfind::lineOfSight(const NavGrid& nav, const Vec3& p1, const Vec3& p2) {
    float dist = distance(p1, p2);
    int steps = static_cast<int>(std::ceil(dist / (nav.cellSize() * 0.4f)));
    if (steps <= 0) return true;

    for (int i = 1; i < steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        Vec3 sample = p1 + (p2 - p1) * t;
        int gx, gz;
        if (!nav.worldToGrid(sample, gx, gz) || !nav.isWalkable(gx, gz)) {
            return false;
        }
    }
    return true;
}

std::vector<Vec3> Pathfind::smoothPath(const NavGrid& nav, const std::vector<Vec3>& rawPath) {
    std::vector<Vec3> smoothed;
    if (rawPath.empty()) return smoothed;

    smoothed.push_back(rawPath[0]);
    size_t curIdx = 0;

    while (curIdx < rawPath.size() - 1) {
        size_t furthestVisible = curIdx + 1;
        for (size_t nextIdx = curIdx + 2; nextIdx < rawPath.size(); ++nextIdx) {
            if (lineOfSight(nav, rawPath[curIdx], rawPath[nextIdx])) {
                furthestVisible = nextIdx;
            } else {
                break;
            }
        }
        smoothed.push_back(rawPath[furthestVisible]);
        curIdx = furthestVisible;
    }

    return smoothed;
}

} // namespace hm
