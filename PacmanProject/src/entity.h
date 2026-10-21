#pragma once

#include <functional>

#include "config.h"

enum class Dir { None = 0, Up, Down, Left, Right };

inline int dx(Dir d) { return d == Dir::Left ? -1 : d == Dir::Right ? 1 : 0; }
inline int dy(Dir d) { return d == Dir::Up ? -1 : d == Dir::Down ? 1 : 0; }

inline Dir opposite(Dir d) {
    switch (d) {
        case Dir::Up: return Dir::Down;
        case Dir::Down: return Dir::Up;
        case Dir::Left: return Dir::Right;
        case Dir::Right: return Dir::Left;
        default: return Dir::None;
    }
}

struct Entity {
    int tr = 0, tc = 0;   // current tile (row, col)
    Dir dir = Dir::None;  // direction of travel
    Dir want = Dir::None; // buffered input direction
    Dir facing = Dir::Left;
    float p = 0.f;        // progress toward next tile center [0,1)
    double speed = 0.0;   // tiles / second

    float fx() const { return (tc + 0.5f + dx(dir) * p) * cfg::kTile; }
    float fy() const { return (tr + 0.5f + dy(dir) * p) * cfg::kTile; }

    void place(int c, int r, Dir d = Dir::None) {
        tc = c;
        tr = r;
        p = 0.f;
        dir = d;
        want = d;
        if (d != Dir::None) facing = d;
    }
};

// Tile-centered movement: direction changes only happen at tile centers,
// walls stop the entity at the center of the last open tile, the tunnel row
// wraps horizontally. `decide` picks the direction at each center crossing.
inline void advance(Entity& e, double dt,
                    const std::function<bool(int, int)>& pass,
                    const std::function<Dir(Entity&)>& decide) {
    double rem = e.speed * dt;
    int guard = 0;
    while (rem > 1e-9 && guard++ < 16) {
        if (e.dir == Dir::None) {
            Dir d = decide(e);
            if (d != Dir::None && pass(e.tr + dy(d), e.tc + dx(d))) {
                e.dir = d;
                e.facing = d;
            } else {
                break;
            }
        }
        if (!pass(e.tr + dy(e.dir), e.tc + dx(e.dir))) {
            e.dir = Dir::None;
            break;
        }
        double toCenter = 1.0 - e.p;
        if (rem < toCenter) {
            e.p += float(rem);
            break;
        }
        rem -= toCenter;
        e.tc += dx(e.dir);
        e.tr += dy(e.dir);
        e.p = 0.f;
        if (e.tc < 0) e.tc += cfg::kCols;
        if (e.tc >= cfg::kCols) e.tc -= cfg::kCols;
        Dir d = decide(e);
        if (d != Dir::None && pass(e.tr + dy(d), e.tc + dx(d))) {
            e.dir = d;
            e.facing = d;
        } else {
            e.dir = Dir::None;
        }
    }
}

// 180-degree turn, valid mid-corridor (used on fright / mode switches).
inline void reverseMid(Entity& e) {
    if (e.dir == Dir::None) {
        e.want = opposite(e.want);
        return;
    }
    if (e.p > 0.f) {
        e.tc += dx(e.dir);
        e.tr += dy(e.dir);
        e.p = 1.f - e.p;
        if (e.tc < 0) e.tc += cfg::kCols;
        if (e.tc >= cfg::kCols) e.tc -= cfg::kCols;
    }
    e.dir = opposite(e.dir);
    e.want = e.dir;
    e.facing = e.dir;
}
