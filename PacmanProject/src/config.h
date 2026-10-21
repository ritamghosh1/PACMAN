#pragma once

#include <algorithm>
#include <cmath>

namespace cfg {

constexpr int kTile = 5;    // 1 grid block = 5px (README)
constexpr int kScale = 4;   // display zoom so the 5px grid is playable
constexpr int kCols = 28;
constexpr int kRows = 31;
constexpr int kW = kCols * kTile;           // 140 logical px
constexpr int kH = kRows * kTile;           // 155 logical px
constexpr int kHud = 16;                    // logical px HUD strip
constexpr int kTunnelRow = 14;
constexpr int kTickMs = 16;
constexpr float kPi = 3.14159265f;

struct Level {
    double pacSpeed;     // tiles per second
    double ghostSpeed;   // tiles per second
    double frightSecs;   // seconds of frightened mode per pellet
    double releaseBase;  // seconds between ghost house releases
    int predictive;      // aim-ahead tiles (AI complexity)
};

inline Level level(int n) {
    if (n < 1) n = 1;
    Level L;
    L.pacSpeed = std::min(8.5, 7.0 + 0.15 * (n - 1));
    L.ghostSpeed = std::min(9.5, 6.4 + 0.45 * (n - 1));
    L.frightSecs = std::max(1.5, 6.0 - 0.6 * (n - 1));
    L.releaseBase = std::max(0.5, 1.6 * std::pow(0.75, n - 1));
    L.predictive = std::min(3, std::max(0, n - 2));
    return L;
}

} // namespace cfg
