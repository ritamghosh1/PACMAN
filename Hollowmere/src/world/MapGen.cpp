#include "MapGen.h"
#include "gfx/MeshBuilder.h"
#include "Props.h"
#include "PropsGore.h"
#include "core/rng.h"
#include "story/Notes.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <queue>

namespace hm {

namespace {

constexpr int GW = 41, GH = 41;
constexpr float CELL = 2.4f;
constexpr float WALL_H = 3.4f;

struct R { int c0, r0, c1, r1; };
bool inR(const R& r, int c, int rr) { return c >= r.c0 && c <= r.c1 && rr >= r.r0 && rr <= r.r1; }

// Wing tints so the player can tell where they are by colour alone.
Vec4 tintFor(int wing) {
    switch (wing) {
    case 1: return Vec4(1.00f, 0.82f, 0.62f, 1);   // library: warm timber
    case 2: return Vec4(0.70f, 1.00f, 0.78f, 1);   // conservatory: green
    case 3: return Vec4(0.62f, 0.72f, 0.92f, 1);   // cellar: cold iron blue
    case 4: return Vec4(1.00f, 0.80f, 0.90f, 1);   // nursery: faded pink
    case 5: return Vec4(0.95f, 0.70f, 0.72f, 1);   // chapel: blood stone
    default: return Vec4(0.95f, 0.92f, 0.88f, 1);
    }
}

} // namespace

Vec3 MapData::cellCenter(int c, int r) const {
    return Vec3(origin.x + c * cellSize + cellSize * 0.5f, 0.0f, origin.z + r * cellSize + cellSize * 0.5f);
}

bool MapData::worldToCell(const Vec3& p, int& c, int& r) const {
    c = static_cast<int>(std::floor((p.x - origin.x) / cellSize));
    r = static_cast<int>(std::floor((p.z - origin.z) / cellSize));
    return c >= 0 && c < gridW && r >= 0 && r < gridH;
}

int MapData::wingAt(const Vec3& p) const {
    int c, r;
    if (!worldToCell(p, c, r)) return -1;
    for (const auto& w : wings)
        if (c >= w.c0 && c <= w.c1 && r >= w.r0 && r <= w.r1) return w.id;
    // door cells, vestibule: attribute to the foyer so the HUD doesn't flicker
    if (grid[r][c] != '#') return 0;
    return -1;
}

const Pickup* MapData::findPickup(int id) const {
    for (const auto& p : pickups) if (p.id() == id) return &p;
    return nullptr;
}

const Door* MapData::findDoor(int id) const {
    for (const auto& d : doors) if (d.id() == id) return &d;
    return nullptr;
}

bool MapGen::generateManor(QOpenGLFunctions_3_3_Core* gl, MapData& out) {
    out.collision.clear();
    out.doors.clear();
    out.pickups.clear();
    out.pointLights.clear();
    out.wings.clear();
    out.questOrder.clear();

    out.gridW = GW; out.gridH = GH; out.cellSize = CELL;
    out.origin = Vec3(-GW * CELL * 0.5f, 0.0f, -GH * CELL * 0.5f);
    out.grid.assign(GH, std::string(GW, '#'));
    auto& g = out.grid;

    Rng rng(0x401EA7ULL);
    auto rnd01 = [&]() { return rng.nextFloat(); };

    const R rect[6] = {
        {15, 15, 25, 25},   // 0 foyer
        {1, 11, 13, 29},    // 1 library
        {27, 11, 39, 29},   // 2 conservatory
        {15, 27, 25, 39},   // 3 boiler cellar
        {27, 31, 39, 39},   // 4 nursery attic
        {15, 1, 25, 11},    // 5 chapel
    };
    static const char* names[6] = {"The Grand Foyer", "The West Library", "The Green Conservatory", "The Boiler Cellar", "The Attic Nursery", "The High Chapel"};
    struct Door0 { int c, r, key, wing; };
    const Door0 doorDefs[5] = {
        {14, 21, 0, 1}, {26, 21, 1, 2}, {21, 26, 2, 3}, {26, 33, 3, 4}, {20, 14, 4, 5},
    };
    const int entranceCell[6][2] = {{20, 20}, {13, 21}, {27, 21}, {21, 27}, {27, 33}, {20, 11}};

    // --- carve foyer & chapel halls ---
    for (int w : {0, 5})
        for (int r = rect[w].r0; r <= rect[w].r1; ++r)
            for (int c = rect[w].c0; c <= rect[w].c1; ++c) g[r][c] = '.';
    g[12][20] = g[13][20] = '.';
    for (auto pc : std::vector<std::pair<int, int>>{{17, 17}, {23, 17}, {17, 23}, {23, 23}, {16, 2}, {24, 2}, {16, 9}, {24, 9}}) g[pc.second][pc.first] = 'P';

    // --- carve the four mazes (recursive backtracker, then braid & open plazas) ---
    for (int w = 1; w <= 4; ++w) {
        const R& rc = rect[w];
        std::vector<std::pair<int, int>> stack;
        int sc = entranceCell[w][0], sr = entranceCell[w][1];
        g[sr][sc] = '.';
        stack.push_back({sc, sr});
        while (!stack.empty()) {
            auto [c, r] = stack.back();
            std::array<std::pair<int, int>, 4> dirs = {{{2, 0}, {-2, 0}, {0, 2}, {0, -2}}};
            for (int i = 3; i > 0; --i) std::swap(dirs[i], dirs[static_cast<int>(rnd01() * (i + 1)) % (i + 1)]);
            bool moved = false;
            for (auto [dc, dr] : dirs) {
                int nc = c + dc, nr = r + dr;
                if (!inR(rc, nc, nr) || g[nr][nc] != '#') continue;
                g[r + dr / 2][c + dc / 2] = '.';
                g[nr][nc] = '.';
                stack.push_back({nc, nr});
                moved = true;
                break;
            }
            if (!moved) stack.pop_back();
        }
        // Braid: knock out some walls so there are loops to run around
        for (int r = rc.r0 + 1; r < rc.r1; ++r)
            for (int c = rc.c0 + 1; c < rc.c1; ++c) {
                if (g[r][c] != '#' || ((c + r) & 1) == 0) continue;
                bool horiz = (r & 1) == 1;   // wall between (c-1,r) and (c+1,r)
                bool a = horiz ? g[r][c - 1] == '.' && g[r][c + 1] == '.' : g[r - 1][c] == '.' && g[r + 1][c] == '.';
                if (a && rnd01() < 0.20f) g[r][c] = '.';
            }
        // Plazas: a few open 3x3 rooms
        for (int k = 0; k < 3; ++k) {
            int pc = rc.c0 + 2 + 2 * static_cast<int>(rnd01() * ((rc.c1 - rc.c0 - 2) / 2));
            int pr = rc.r0 + 2 + 2 * static_cast<int>(rnd01() * ((rc.r1 - rc.r0 - 2) / 2));
            if (pc < rc.c0 + 2 || pc > rc.c1 - 2 || pr < rc.r0 + 2 || pr > rc.r1 - 2) continue;
            for (int r = pr - 1; r <= pr + 1; ++r) for (int c = pc - 1; c <= pc + 1; ++c) g[r][c] = '.';
        }
    }

    // --- doors ---
    int doorCounter = 1;
    std::vector<std::pair<int, int>> doorCells;
    for (const auto& d : doorDefs) {
        g[d.r][d.c] = d.key ? 'L' : 'D';
        doorCells.push_back({d.c, d.r});
    }

    // --- wings metadata ---
    out.wings.resize(6);
    for (int w = 0; w < 6; ++w) {
        WingInfo& wi = out.wings[w];
        wi.id = w;
        wi.name = names[w];
        wi.c0 = rect[w].c0; wi.r0 = rect[w].r0; wi.c1 = rect[w].c1; wi.r1 = rect[w].r1;
        wi.entrance = out.cellCenter(entranceCell[w][0], entranceCell[w][1]);
        wi.safeCenter = (w == 0) ? out.cellCenter(20, 20) : wi.entrance;
        wi.safeRadius = (w == 0) ? 9.0f : 2.5f;
    }
    out.playerSpawnPos = out.cellCenter(20, 23) + Vec3(0, 0.1f, 0);

    // --- BFS distances inside each wing ---
    auto bfs = [&](int w) {
        std::vector<std::vector<int>> dist(GH, std::vector<int>(GW, -1));
        std::queue<std::pair<int, int>> q;
        q.push({entranceCell[w][0], entranceCell[w][1]});
        dist[entranceCell[w][1]][entranceCell[w][0]] = 0;
        while (!q.empty()) {
            auto [c, r] = q.front(); q.pop();
            for (auto [dc, dr] : std::array<std::pair<int, int>, 4>{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}}) {
                int nc = c + dc, nr = r + dr;
                if (!inR(rect[w], nc, nr) || g[nr][nc] == '#' || g[nr][nc] == 'P' || dist[nr][nc] >= 0) continue;
                dist[nr][nc] = dist[r][c] + 1;
                q.push({nc, nr});
            }
        }
        return dist;
    };

    int pickupId = 1;
    std::vector<std::pair<int, int>> usedCells;
    auto addPickup = [&](PickupType t, int c, int r, const QString& name, const QString& desc, int tag, float y = 0.55f) {
        Vec3 p = out.cellCenter(c, r);
        p.y = y;
        out.pickups.emplace_back(pickupId, t, p, name, desc);
        out.pickups.back().setTag(tag);
        usedCells.push_back({c, r});
        return pickupId++;
    };

    static const char* sigilNames[5] = {"", "Ada's Crimson Ribbon", "The Tooth Ring", "The Bent Tuning Fork", "Eli's Blanket"};
    static const char* sigilDesc[5] = {"",
        "Ada's ribbon, stiff with old tears. She was punished for crying. Memory Sigil 1 of 4.",
        "A child's tooth threaded on a ring, taken from a flowerbed that was a grave. Memory Sigil 2 of 4.",
        "A tuning fork bent like a spine; it still hums the note the Chorus Ward was made to sing. Memory Sigil 3 of 4.",
        "A small blanket that still smells of smoke. Eli never let go of it. Memory Sigil 4 of 4."};
    static const char* keyDesc[5] = {"", "A green-tagged brass key marked 'Conservatory'.", "A rust-red key marked 'Boiler Cellar'.", "A small key on a ribbon, marked 'Nursery'.", "A tall black key with a crow's head, marked 'Chapel'."};
    static const int wingNotes[6][4] = {{3, 6, 0, 0}, {8, 9, 10, 12}, {16, 18, 20, 22}, {25, 28, 29, 30}, {31, 34, 35, 36}, {38, 39, 0, 0}};
    const auto allNotes = NoteLibrary::createAllNotes();
    auto noteById = [&](int id) -> const NoteData* { for (const auto& n : allNotes) if (n.id == id) return &n; return nullptr; };
    auto addNote = [&](int id, int c, int r) {
        const NoteData* n = noteById(id);
        if (!n) return;
        addPickup(PickupType::Note, c, r, QString::fromStdString(n->title), QString::fromStdString(n->text), id, 0.9f);
    };

    for (int w = 1; w <= 4; ++w) {
        auto dist = bfs(w);
        struct Cand { int c, r, d; };
        std::vector<Cand> cands;
        int maxD = 1;
        for (int r = rect[w].r0; r <= rect[w].r1; ++r)
            for (int c = rect[w].c0; c <= rect[w].c1; ++c)
                if (dist[r][c] > 3 && (c & 1) && (r & 1)) { cands.push_back({c, r, dist[r][c]}); maxD = std::max(maxD, dist[r][c]); }
        std::sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b) { return a.d > b.d; });
        auto take = [&](float frac, int minSep = 3) -> Cand {
            int start = std::clamp(static_cast<int>(frac * cands.size()), 0, static_cast<int>(cands.size()) - 1);
            for (int k = 0; k < static_cast<int>(cands.size()); ++k) {
                const Cand& cd = cands[(start + k) % cands.size()];
                bool ok = true;
                for (auto& u : usedCells) if (std::abs(u.first - cd.c) + std::abs(u.second - cd.r) < minSep) { ok = false; break; }
                if (ok) return cd;
            }
            return cands[start];
        };
        Cand sig = take(0.0f);
        int sigId = addPickup(PickupType::MemorySigil, sig.c, sig.r, sigilNames[w], sigilDesc[w], w, 0.75f);
        Cand key = take(0.22f, 7);
        int keyId = addPickup(PickupType::Key, key.c, key.r, Door::keyName(w), keyDesc[w], w, 0.55f);
        out.questOrder.push_back(sigId);
        out.questOrder.push_back(keyId);
        Cand oil = take(0.5f); addPickup(PickupType::OilFlask, oil.c, oil.r, "Lantern Oil Flask", "Restores kerosene to your lantern.", 0, 0.4f);
        if (w >= 2) { Cand bd = take(0.38f); addPickup(PickupType::Bandage, bd.c, bd.r, "Sterile Bandages", "Binds wounds. Heals one level of damage.", 0, 0.4f); }
        for (int k = 0; k < 4; ++k) { Cand n = take(0.14f + 0.2f * k, 4); addNote(wingNotes[w][k], n.c, n.r); }
        // Ghost home and patrol loop
        Cand home = take(0.55f, 2);
        out.wings[w].ghostSpawn = out.cellCenter(home.c, home.r);
        for (float f : {0.30f, 0.65f, 0.85f, 0.45f}) { Cand pc = cands[std::clamp(static_cast<int>(f * cands.size()), 0, static_cast<int>(cands.size()) - 1)]; out.wings[w].patrol.push_back(out.cellCenter(pc.c, pc.r)); }
    }
    // Foyer and chapel notes
    addNote(3, 17, 20); addNote(6, 23, 20);
    addNote(38, 17, 7); addNote(39, 23, 7);
    // Oil flask to start with in the foyer so the lantern tutorial has a reward
    addPickup(PickupType::OilFlask, 21, 17, "Lantern Oil Flask", "Restores kerosene to your lantern.", 0, 0.4f);
    out.altarPos = out.cellCenter(20, 3);
    out.wardenSpawn = out.cellCenter(20, 6);
    out.wings[5].ghostSpawn = out.wardenSpawn;

    // --- geometry ---
    MeshBuilder wallB, floorB, ceilB, propsB;
    auto wingOf = [&](int c, int r) {
        for (int w = 5; w >= 0; --w) if (inR(rect[w], c, r)) return w;
        return 0;
    };
    auto isFloor = [&](int c, int r) { return c >= 0 && c < GW && r >= 0 && r < GH && g[r][c] != '#'; };

    for (int r = 0; r < GH; ++r) {
        for (int c = 0; c < GW; ++c) {
            Vec3 ctr = out.cellCenter(c, r);
            float x = ctr.x, z = ctr.z, h = CELL * 0.5f;
            char t = g[r][c];
            int w = wingOf(c, r);
            Vec4 tint = tintFor(w);
            if (t != '#') {
                floorB.addQuad(Vec3(x - h, 0, z + h), Vec3(x + h, 0, z + h), Vec3(x + h, 0, z - h), Vec3(x - h, 0, z - h),
                               Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1), Vec3(0, 1, 0), tint);
                ceilB.addQuad(Vec3(x - h, WALL_H, z - h), Vec3(x + h, WALL_H, z - h), Vec3(x + h, WALL_H, z + h), Vec3(x - h, WALL_H, z + h),
                              Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1), Vec3(0, -1, 0), tint);
            }
            if (t == '#') {
                bool visible = false;
                for (int dr = -1; dr <= 1 && !visible; ++dr) for (int dc = -1; dc <= 1; ++dc) if (isFloor(c + dc, r + dr)) { visible = true; break; }
                if (!visible) continue;
                // tint by the neighbouring floor's wing
                int nw = w;
                for (int dr = -1; dr <= 1; ++dr) for (int dc = -1; dc <= 1; ++dc) if (isFloor(c + dc, r + dr)) nw = wingOf(c + dc, r + dr);
                wallB.addBox(Vec3(x, WALL_H * 0.5f, z), Vec3(CELL, WALL_H, CELL), Vec2(1.0f, 1.5f), tintFor(nw));
                out.collision.addBox(AABB(Vec3(x - h, 0, z - h), Vec3(x + h, WALL_H, z + h)), 0, 0);
            } else if (t == 'P') {
                propsB.addCylinder(Vec3(x, 0, z), 0.38f, WALL_H, 14, Vec4(0.85f, 0.85f, 0.85f, 1));
                propsB.addBox(Vec3(x, 0.2f, z), Vec3(0.9f, 0.4f, 0.9f));
                propsB.addBox(Vec3(x, WALL_H - 0.2f, z), Vec3(0.9f, 0.4f, 0.9f));
                out.collision.addBox(AABB(Vec3(x - 0.42f, 0, z - 0.42f), Vec3(x + 0.42f, WALL_H, z + 0.42f)), 0, 0);
            } else if (t == 'D' || t == 'L') {
                int key = 0, wingId = 0;
                for (const auto& d : doorDefs) if (d.c == c && d.r == r) { key = d.key; wingId = d.wing; }
                bool spansX = !isFloor(c - 1, r) && !isFloor(c + 1, r);   // passage runs north-south
                Door door(doorCounter++, Vec3(x, 0, z), spansX, key, CELL);
                door.setInnerPoint(out.cellCenter(entranceCell[wingId][0], entranceCell[wingId][1]));
                out.wings[wingId].doorId = door.id();
                out.doors.push_back(door);
                out.collision.addBox(door.currentColliderBox(), 1, door.id());
                wallB.addBox(Vec3(x, 2.6f + 0.4f, z), Vec3(CELL, 0.8f, CELL), Vec2(1, 0.5f), tint);   // lintel
            }
        }
    }

    // --- sconces & lights (lit candle on walls of corridor-ish cells) ---
    auto addCandleLight = [&](const Vec3& pos, float intensity, float range, const Vec3& col) {
        PointLight pl;
        pl.position = pos; pl.color = col; pl.intensity = intensity; pl.range = range;
        out.pointLights.push_back(pl);
        propsB.addCylinder(Vec3(pos.x, pos.y - 0.35f, pos.z), 0.07f, 0.3f, 8, Vec4(0.3f, 0.3f, 0.3f, 1));
        propsB.addSphere(pos + Vec3(0, 0.02f, 0), 0.06f, 6, 8, Vec4(1.0f, 0.8f, 0.2f, 1));
    };
    struct Dir { int dc, dr; };
    const Dir dirs4[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int r = 0; r < GH; ++r)
        for (int c = 0; c < GW; ++c) {
            if (g[r][c] != '.') continue;
            int w = wingOf(c, r);
            float prob = (w == 0 || w == 5) ? 0.10f : 0.075f;
            if (rnd01() > prob) continue;
            for (const Dir& d : dirs4) {
                if (g[r + d.dr][c + d.dc] != '#') continue;
                Vec3 cc = out.cellCenter(c, r);
                addCandleLight(Vec3(cc.x + d.dc * (CELL * 0.5f - 0.1f), 2.0f, cc.z + d.dr * (CELL * 0.5f - 0.1f)), 1.3f, 8.0f, Vec3(1.0f, 0.65f, 0.35f));
                break;
            }
        }
    // Safe-room candelabras: a steadier, warmer glow at each wing entrance, plus the foyer chandelier
    for (int w = 1; w <= 5; ++w) {
        Vec3 e = out.wings[w].entrance;
        Props::buildCandelabra(propsB, Vec3(e.x, 0, e.z));
        PointLight pl; pl.position = Vec3(e.x, 1.7f, e.z); pl.color = Vec3(1.0f, 0.78f, 0.5f); pl.intensity = 1.8f; pl.range = 9.0f;
        out.pointLights.push_back(pl);
    }
    { PointLight pl; pl.position = Vec3(0, 3.0f, out.cellCenter(20, 20).z); pl.color = Vec3(1.0f, 0.82f, 0.6f); pl.intensity = 3.0f; pl.range = 24.0f; out.pointLights.push_back(pl); }
    for (auto corner : std::vector<std::pair<int, int>>{{16, 16}, {24, 16}, {16, 24}, {24, 24}}) {
        Vec3 cc = out.cellCenter(corner.first, corner.second);
        PointLight pl; pl.position = Vec3(cc.x, 2.4f, cc.z); pl.color = Vec3(1.0f, 0.7f, 0.42f); pl.intensity = 1.5f; pl.range = 11.0f;
        out.pointLights.push_back(pl);
    }
    { PointLight pl; pl.position = Vec3(out.altarPos.x, 2.2f, out.altarPos.z + 1.2f); pl.color = Vec3(1.0f, 0.5f, 0.35f); pl.intensity = 2.0f; pl.range = 12.0f; out.pointLights.push_back(pl); }

    // --- decor per wing ---
    auto solid = [&](float x0, float z0, float x1, float z1, float hgt) {
        out.collision.addBox(AABB(Vec3(x0, 0, z0), Vec3(x1, hgt, z1)), 0, 0);
    };
    for (int r = 1; r < GH - 1; ++r)
        for (int c = 1; c < GW - 1; ++c) {
            if (g[r][c] != '.') continue;
            int w = wingOf(c, r);
            if (w < 1 || w > 4) continue;
            Vec3 cc = out.cellCenter(c, r);
            bool near = false;
            for (auto& u : usedCells) if (std::abs(u.first - c) + std::abs(u.second - r) < 2) near = true;
            for (auto& dc : doorCells) if (std::abs(dc.first - c) + std::abs(dc.second - r) < 3) near = true;
            if (std::abs(entranceCell[w][0] - c) + std::abs(entranceCell[w][1] - r) < 3) near = true;
            if (near) continue;
            // Only decorate cells with exactly one neighbouring wall so corridors stay passable
            int walls = 0; Dir wd{0, 0};
            for (const Dir& d : dirs4) if (g[r + d.dr][c + d.dc] == '#') { ++walls; wd = d; }
            if (walls != 1) continue;
            float roll = rnd01();
            float off = CELL * 0.5f - 0.28f;
            Vec3 pos(cc.x + wd.dc * off, 0, cc.z + wd.dr * off);
            bool alongX = wd.dc != 0;   // wall is east/west => shelf runs along Z
            if (w == 1 && roll < 0.55f) {
                Vec3 sz = alongX ? Vec3(0.5f, 2.5f, 1.9f) : Vec3(1.9f, 2.5f, 0.5f);
                propsB.addBox(pos + Vec3(0, 1.25f, 0), sz, Vec2(1, 1), Vec4(0.32f, 0.20f, 0.12f, 1));
                for (int s = 0; s < 4; ++s) {
                    Vec4 bc = (s + c) % 3 == 0 ? Vec4(0.5f, 0.15f, 0.15f, 1) : ((s + c) % 3 == 1 ? Vec4(0.15f, 0.25f, 0.5f, 1) : Vec4(0.25f, 0.38f, 0.2f, 1));
                    Vec3 bsz = alongX ? Vec3(0.12f, 0.38f, 1.7f) : Vec3(1.7f, 0.38f, 0.12f);
                    propsB.addBox(pos + Vec3(-wd.dc * 0.26f, 0.45f + s * 0.6f, -wd.dr * 0.26f), bsz, Vec2(1, 1), bc);
                }
                solid(pos.x - sz.x * 0.5f, pos.z - sz.z * 0.5f, pos.x + sz.x * 0.5f, pos.z + sz.z * 0.5f, 2.5f);
            } else if (w == 2 && roll < 0.5f) {
                propsB.addCylinder(pos, 0.30f, 0.45f, 10, Vec4(0.45f, 0.25f, 0.15f, 1));
                for (int k = 0; k < 4; ++k)
                    propsB.addSphere(pos + Vec3((k - 1.5f) * 0.14f, 0.85f + 0.12f * (k % 2), (k % 2 ? 0.1f : -0.1f)), 0.34f, 6, 8, Vec4(0.12f + 0.05f * k, 0.5f, 0.2f, 1));
                solid(pos.x - 0.34f, pos.z - 0.34f, pos.x + 0.34f, pos.z + 0.34f, 1.6f);
            } else if (w == 3 && roll < 0.5f) {
                propsB.addCylinder(pos, 0.40f, 0.95f, 12, Vec4(0.30f, 0.20f, 0.12f, 1));
                propsB.addCylinder(pos + Vec3(0, 0.45f, 0), 0.42f, 0.05f, 12, Vec4(0.15f, 0.15f, 0.15f, 1));
                solid(pos.x - 0.42f, pos.z - 0.42f, pos.x + 0.42f, pos.z + 0.42f, 1.0f);
            } else if (w == 4 && roll < 0.5f) {
                Vec3 sz = alongX ? Vec3(1.0f, 0.55f, 1.8f) : Vec3(1.8f, 0.55f, 1.0f);
                propsB.addBox(pos + Vec3(-wd.dc * 0.25f, 0.4f, -wd.dr * 0.25f), sz, Vec2(1, 1), Vec4(0.85f, 0.8f, 0.78f, 1));
                for (int k = -2; k <= 2; ++k) {
                    Vec3 bp = pos + Vec3(-wd.dc * 0.25f, 0.8f, -wd.dr * 0.25f) + (alongX ? Vec3(0.45f, 0, k * 0.36f) : Vec3(k * 0.36f, 0, 0.45f));
                    propsB.addBox(bp, Vec3(0.03f, 0.5f, 0.03f), Vec2(1, 1), Vec4(0.9f, 0.88f, 0.85f, 1));
                }
                solid(pos.x - sz.x * 0.5f - wd.dc * 0.25f, pos.z - sz.z * 0.5f - wd.dr * 0.25f, pos.x + sz.x * 0.5f - wd.dc * 0.25f, pos.z + sz.z * 0.5f - wd.dr * 0.25f, 0.9f);
            }
        }

    // Foyer furniture, mirror & clock
    Vec3 fo = out.cellCenter(20, 15);
    Props::buildGrandfatherClock(propsB, Vec3(fo.x - 4.0f, 0, fo.z - 0.6f));
    solid(fo.x - 4.4f, fo.z - 0.9f, fo.x - 3.6f, fo.z - 0.2f, 2.6f);
    Props::buildMirrorPedestal(propsB, out.cellCenter(24, 24) + Vec3(0, 0, 0));
    solid(out.cellCenter(24, 24).x - 0.3f, out.cellCenter(24, 24).z - 0.3f, out.cellCenter(24, 24).x + 0.3f, out.cellCenter(24, 24).z + 0.3f, 1.8f);
    // Cellar: steam pipes overhead & a gurney set-piece in the nursery-side plaza
    Props::buildSteamPipeRun(propsB, out.cellCenter(16, 33) + Vec3(0, 3.0f, 0), out.cellCenter(24, 33) + Vec3(0, 3.0f, 0));
    // Chapel: altar and pews
    Props::buildAltarTable(propsB, Vec3(out.altarPos.x, 0, out.altarPos.z - 0.5f));
    solid(out.altarPos.x - 1.4f, out.altarPos.z - 1.2f, out.altarPos.x + 1.4f, out.altarPos.z + 0.3f, 1.6f);
    for (int row = 0; row < 3; ++row) {
        float pz = out.cellCenter(20, 6).z + row * 2.5f + 2.0f;
        for (int side : {-1, 1}) {
            Vec3 pp(out.cellCenter(20, 6).x + side * 4.0f, 0, pz);
            Props::buildPew(propsB, pp);
            solid(pp.x - 1.45f, pp.z - 0.35f, pp.x + 1.45f, pp.z + 0.35f, 1.0f);
        }
    }

    wallB.build(gl, out.wallMesh);
    floorB.build(gl, out.floorMesh);
    ceilB.build(gl, out.ceilingMesh);
    propsB.build(gl, out.propsMesh);
    return true;
}

} // namespace hm
