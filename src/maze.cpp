#include "maze.h"

#include <algorithm>
#include <cstring>

#include "config.h"

namespace {

// 28 x 31 classic-style layout. Row 14 is the wrap-around tunnel.
const char* kLayout[cfg::kRows] = {
    "############################", //  0
    "#............##............#", //  1
    "#.####.#####.##.#####.####.#", //  2
    "#o####.#####.##.#####.####o#", //  3
    "#.####.#####.##.#####.####.#", //  4
    "#..........................#", //  5
    "#.####.##.########.##.####.#", //  6
    "#.####.##.########.##.####.#", //  7
    "#......##....##....##......#", //  8
    "######.##### ## #####.######", //  9
    "######.##### ## #####.######", // 10
    "######.##          ##.######", // 11
    "######.## ###--### ##.######", // 12  door cols 13-14
    "######.## #      # ##.######", // 13
    "          #      #          ", // 14  tunnel
    "######.## #      # ##.######", // 15
    "######.## ######## ##.######", // 16
    "######.##          ##.######", // 17
    "######.## ######## ##.######", // 18
    "######.## ######## ##.######", // 19
    "#............##............#", // 20
    "#.####.#####.##.#####.####.#", // 21
    "#.####.#####.##.#####.####.#", // 22
    "#o..##.......  .......##..o#", // 23  pacman spawn cols 13-14
    "###.##.##.########.##.##.###", // 24
    "###.##.##.########.##.##.###", // 25
    "#......##....##....##......#", // 26
    "#.##########.##.##########.#", // 27
    "#.##########.##.##########.#", // 28
    "#..........................#", // 29
    "############################", // 30
};

} // namespace

void Maze::reset() {
    g_.assign(kLayout, kLayout + cfg::kRows);
    dots_.clear();
    pellets_.clear();
    for (int r = 0; r < cfg::kRows; r++) {
        for (int c = 0; c < cfg::kCols; c++) {
            if (g_[r][c] == '.') dots_.push_back(QPoint(c, r));
            else if (g_[r][c] == 'o') pellets_.push_back(QPoint(c, r));
        }
    }
}

char Maze::at(int r, int c) const {
    if (r >= 0 && r < cfg::kRows) {
        if (c < 0 || c >= cfg::kCols) {
            if (r == cfg::kTunnelRow) return g_[r][(c + cfg::kCols) % cfg::kCols];
            return '#';
        }
        return g_[r][c];
    }
    return '#';
}

bool Maze::walkablePac(int r, int c) const {
    if (interiorTile(r, c)) return false;
    char ch = at(r, c);
    return ch != '#' && ch != '-';
}

bool Maze::walkableGhost(int r, int c, bool doorOk, bool interiorOk) const {
    if (interiorTile(r, c)) return interiorOk && at(r, c) != '#';
    char ch = at(r, c);
    if (ch == '-') return doorOk;
    return ch != '#';
}

int Maze::eat(int r, int c) {
    if (r < 0 || r >= cfg::kRows) return 0;
    if (c < 0 || c >= cfg::kCols) {
        if (r == cfg::kTunnelRow) c = (c + cfg::kCols) % cfg::kCols;
        else return 0;
    }
    char& ch = g_[r][c];
    int pts = 0;
    if (ch == '.') pts = 10;
    else if (ch == 'o') pts = 50;
    if (!pts) return 0;
    ch = ' ';
    auto kill = [&](std::vector<QPoint>& v) {
        v.erase(std::remove_if(v.begin(), v.end(),
                                [&](const QPoint& q) { return q.x() == c && q.y() == r; }),
                v.end());
    };
    if (pts == 10) kill(dots_);
    else kill(pellets_);
    return pts;
}
