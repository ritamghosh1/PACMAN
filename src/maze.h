#pragma once

#include <QPoint>
#include <string>
#include <vector>

// '#' wall, '.' dot, 'o' power pellet, ' ' floor, '-' ghost house door.
class Maze {
public:
    void reset();  // reload layout, rebuild dot/pellet lists
    char at(int r, int c) const;
    bool walkablePac(int r, int c) const;
    bool walkableGhost(int r, int c, bool doorOk, bool interiorOk) const;
    int eat(int r, int c);  // 0 / 10 / 50
    bool empty() const { return dots_.empty() && pellets_.empty(); }
    const std::vector<QPoint>& dots() const { return dots_; }
    const std::vector<QPoint>& pellets() const { return pellets_; }

    static bool doorTile(int r, int c) { return r == 12 && (c == 13 || c == 14); }
    static bool interiorTile(int r, int c) {
        return r >= 13 && r <= 15 && c >= 11 && c <= 16;
    }

private:
    std::vector<std::string> g_;
    std::vector<QPoint> dots_, pellets_;
};
