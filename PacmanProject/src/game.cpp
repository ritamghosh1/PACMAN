#include "game.h"
#include "pixel_graphics.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QRandomGenerator>
#include <QSettings>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <queue>

namespace {

struct ModeSeg {
    bool chase;
    double dur;
};

const ModeSeg kModes[] = {
    {false, 7.0}, {true, 20.0}, {false, 7.0}, {true, 20.0}, {false, 5.0}, {true, 1e9},
};
constexpr int kModeCount = 6;

const QPoint kCorners[4] = {QPoint(25, 0), QPoint(2, 0), QPoint(27, 30), QPoint(0, 30)};
const int kSpawnCol[4] = {13, 14, 12, 15};
const double kRelease[4] = {0.0, 1.0, 3.0, 5.0};
const QColor kGhostCol[4] = {
    QColor(255, 0, 0), QColor(255, 184, 255), QColor(0, 255, 255), QColor(255, 184, 82),
};

// Per-level maze colour schemes (cycled one per level).
struct Theme {
    QRgb outer;  // wall edge
    QRgb inner;  // wall shadow ring
    QRgb dot;    // dots and power pellets
};
const Theme kThemes[] = {
    {qRgb(33, 33, 222),  qRgb(10, 10, 80),  qRgb(255, 183, 174)},  // classic blue
    {qRgb(255, 80, 140), qRgb(90, 10, 45),  qRgb(255, 255, 255)},  // pink
    {qRgb(255, 140, 30), qRgb(85, 40, 0),   qRgb(120, 220, 255)},  // orange
    {qRgb(0, 200, 200),  qRgb(0, 60, 70),   qRgb(255, 255, 120)},  // cyan
    {qRgb(60, 200, 60),  qRgb(10, 60, 10),  qRgb(255, 183, 174)},  // green
    {qRgb(170, 80, 255), qRgb(50, 10, 90),  qRgb(255, 255, 255)},  // purple
};
constexpr int kThemeCount = sizeof(kThemes) / sizeof(kThemes[0]);

const Theme& themeForLevel(int level) { return kThemes[(level - 1) % kThemeCount]; }
// New maze every two levels: 1-2 classic, 3-4 grid, 5-6 pillars, 7-8 classic, ...
int layoutForLevel(int level) { return ((level - 1) / 2) % Maze::layoutCount(); }

bool normTile(int r, int c, int& rr, int& cc) {
    if (r < 0 || r >= cfg::kRows) return false;
    cc = c;
    if (cc < 0 || cc >= cfg::kCols) {
        if (r == cfg::kTunnelRow) cc = (cc + cfg::kCols) % cfg::kCols;
        else return false;
    }
    rr = r;
    return true;
}

} // namespace

Game::Game(QWidget* parent)
    : QWidget(parent), buffer_(cfg::kW, cfg::kH + cfg::kHud, QImage::Format_RGB32) {
    setWindowTitle(QStringLiteral("PACMAN - RETRO ARCADE"));
    setFixedSize(cfg::kW * cfg::kScale, (cfg::kH + cfg::kHud) * cfg::kScale);
    setFocusPolicy(Qt::StrongFocus);

    QSettings settings("PacmanArcade", "Pacman");
    highScore_ = settings.value("highScore", 10000).toInt();

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &Game::tick);
    maze_.reset();
    st_ = S::Menu;  // start on the mode-select screen
    clock_.start();
    timer_->start(cfg::kTickMs);
}

void Game::newGame() {
    players_.clear();
    const int count = (mode_ == Mode::Coop) ? 2 : 1;
    for (int i = 0; i < count; i++) {
        Player p;
        p.color = (i == 0) ? QColor(255, 255, 0) : QColor(80, 255, 80);  // P1 yellow, P2 green
        players_.push_back(p);
    }
    ghostScore_ = 0;
    winner_ = -1;
    level_ = 1;
    paused_ = false;
    dotsEatenCount_ = 0;
    fruitActive_ = false;
    fruitT_ = 0.0;
    popups_.clear();
    applyTuning();
    maze_.reset(layoutForLevel(level_));
    resetActors();
    st_ = S::Ready;
    stateT_ = 4.2;
    sound_.playIntro();
}

void Game::applyTuning() {
    cfg::Level L = cfg::level(level_);
    pacSpeed_ = L.pacSpeed;
    ghostSpeed_ = L.ghostSpeed;
    frightSecs_ = L.frightSecs;
    releaseBase_ = L.releaseBase;
    predictive_ = L.predictive;
}

// P1 spawns left of centre facing left, P2 (co-op) right of centre facing right.
void Game::spawnPlayer(int i) {
    Player& pl = players_[i];
    if (players_.size() == 1) pl.pac.place(13, 23, Dir::Left);
    else if (i == 0) pl.pac.place(11, 23, Dir::Left);
    else pl.pac.place(16, 23, Dir::Right);
}

void Game::resetActors() {
    for (size_t i = 0; i < players_.size(); i++) {
        Player& pl = players_[i];
        // A co-op death still animating when the board resets (e.g. the other
        // player just cleared the level) still costs that life.
        if (pl.deathT > 0) {
            pl.deathT = 0;
            pl.lives--;
        }
        pl.respawnT = 0;
        spawnPlayer(int(i));
    }
    ghosts_.assign(4, Ghost{});
    for (int i = 0; i < 4; i++) {
        Ghost& g = ghosts_[i];
        g.id = i;
        g.corner = kCorners[i];
        g.place(kSpawnCol[i], 14);
        g.st = Ghost::St::House;
        g.houseT = float(kRelease[i] * (releaseBase_ / 1.6));
    }
    frightT_ = 0.0;
    frightChain_ = 0;
    modeIdx_ = 0;
    chase_ = false;
    modeT_ = kModes[0].dur;
}

void Game::addScore(int player, int n) {
    Player& p = players_[player];
    p.score += n;
    if (p.score > highScore_) {
        highScore_ = p.score;
        QSettings settings("PacmanArcade", "Pacman");
        settings.setValue("highScore", highScore_);
    }
    if (!p.extraAwarded && p.score >= 10000) {
        p.extraAwarded = true;
        p.lives++;
    }
}

// Ghosts chase whichever living Pac-Man is closest (co-op), or the only one.
const Entity& Game::nearestPac(const Entity& from) const {
    const Entity* best = &players_[0].pac;
    int bestD = -1;
    for (const Player& p : players_) {
        if (!p.active() || p.deathT > 0) continue;  // ignore players who are out or mid-death
        int d = std::abs(p.pac.tc - from.tc) + std::abs(p.pac.tr - from.tr);
        if (bestD < 0 || d < bestD) {
            bestD = d;
            best = &p.pac;
        }
    }
    return *best;
}

void Game::triggerFright() {
    frightT_ = frightSecs_;
    frightChain_ = 0;
    for (auto& g : ghosts_) {
        if (g.st == Ghost::St::Hunting) {
            g.st = Ghost::St::Frightened;
            if (!humanGhost(g)) reverseMid(g);  // never yank the player's ghost around
        }
    }
}

void Game::advanceMode() {
    modeIdx_ = std::min(modeIdx_ + 1, kModeCount - 1);
    chase_ = kModes[modeIdx_].chase;
    modeT_ = kModes[modeIdx_].dur;
    for (auto& g : ghosts_)
        if (g.st == Ghost::St::Hunting && !humanGhost(g)) reverseMid(g);
}

bool Game::ghostPass(const Ghost& g, int r, int c) const {
    bool eyes = g.st == Ghost::St::Eyes || g.st == Ghost::St::Exiting;
    return maze_.walkableGhost(r, c, eyes, eyes || g.st == Ghost::St::House);
}

Dir Game::pacDecide(Entity& e) {
    if (e.want != Dir::None && maze_.walkablePac(e.tr + dy(e.want), e.tc + dx(e.want)))
        return e.want;
    if (e.dir != Dir::None) return e.dir;
    return Dir::None;
}

QPoint Game::ghostTarget(const Ghost& g) const {
    const Entity& pac = nearestPac(g);
    int pr = dy(pac.facing), pc = dx(pac.facing);
    QPoint aim(pac.tc + pc * predictive_, pac.tr + pr * predictive_);
    int dotsLeft = static_cast<int>(maze_.dots().size() + maze_.pellets().size());
    bool elroyActive = (g.id == 0 && dotsLeft <= 20);
    if (!chase_ && !elroyActive) return g.corner;
    switch (g.id) {
        case 1: return aim + QPoint(pc, pr) * 4;
        case 2: {
            QPoint mid = aim + QPoint(pc, pr) * 2;
            QPoint blinky(ghosts_[0].tc, ghosts_[0].tr);
            return mid + (mid - blinky);
        }
        case 3: {
            int d = std::abs(g.tc - aim.x()) + std::abs(g.tr - aim.y());
            if (d > 8) return aim;
            return g.corner;
        }
        default: return aim;
    }
}

Dir Game::bfsStep(const Ghost& g, QPoint target) const {
    static const Dir order[4] = {Dir::Up, Dir::Left, Dir::Down, Dir::Right};
    int dist[cfg::kRows][cfg::kCols];
    std::memset(dist, -1, sizeof(dist));
    int sr, sc;
    if (!normTile(target.y(), target.x(), sr, sc)) return Dir::None;
    if (!maze_.walkableGhost(sr, sc, true, true)) return Dir::None;
    std::queue<QPoint> q;
    dist[sr][sc] = 0;
    q.push(QPoint(sc, sr));
    while (!q.empty()) {
        QPoint cur = q.front();
        q.pop();
        int d0 = dist[cur.y()][cur.x()];
        for (Dir d : order) {
            int rr, cc;
            if (!normTile(cur.y() + dy(d), cur.x() + dx(d), rr, cc)) continue;
            if (dist[rr][cc] >= 0) continue;
            if (!maze_.walkableGhost(rr, cc, true, true)) continue;
            dist[rr][cc] = d0 + 1;
            q.push(QPoint(cc, rr));
        }
    }
    int cur = dist[g.tr][g.tc];
    if (cur <= 0) return Dir::None;
    Dir best = Dir::None;
    int bestD = cur;
    for (int pass = 0; pass < 2; pass++) {
        for (Dir d : order) {
            if (pass == 0 && d == opposite(g.dir)) continue;
            int rr, cc;
            if (!normTile(g.tr + dy(d), g.tc + dx(d), rr, cc)) continue;
            if (dist[rr][cc] < 0) continue;
            if (dist[rr][cc] < bestD) {
                bestD = dist[rr][cc];
                best = d;
            }
        }
        if (best != Dir::None) break;
    }
    return best;
}

Dir Game::ghostDecide(Entity& e) {
    Ghost& g = static_cast<Ghost&>(e);

    if (g.st == Ghost::St::House) return Dir::None;

    if (g.st == Ghost::St::Eyes) {
        if (g.tc == 13 && g.tr == 14) {
            g.st = Ghost::St::House;
            g.houseT = 1.0f;
            return Dir::None;
        }
        return bfsStep(g, QPoint(13, 14));
    }

    if (g.st == Ghost::St::Exiting && g.tr > 11) {
        int align = (g.tc <= 13) ? 13 : 14;
        if (g.tc == align) return Dir::Up;
        return (g.tc < align) ? Dir::Right : Dir::Left;
    }

    if (g.st == Ghost::St::Exiting)
        g.st = (frightT_ > 0.0) ? Ghost::St::Frightened : Ghost::St::Hunting;

    // Versus: P2 steers Blinky whenever it is out roaming (hunting or frightened).
    // Leaving the house and returning as eyes stay automatic.
    if (humanGhost(g)) {
        if (g.want != Dir::None && ghostPass(g, g.tr + dy(g.want), g.tc + dx(g.want)))
            return g.want;
        if (g.dir != Dir::None && ghostPass(g, g.tr + dy(g.dir), g.tc + dx(g.dir)))
            return g.dir;
        return Dir::None;
    }

    if (g.st == Ghost::St::Frightened) {
        static const Dir order[4] = {Dir::Up, Dir::Left, Dir::Down, Dir::Right};
        Dir opts[4];
        int n = 0;
        for (Dir d : order) {
            if (d == opposite(g.dir)) continue;
            if (ghostPass(g, g.tr + dy(d), g.tc + dx(d))) opts[n++] = d;
        }
        if (n == 0) {
            Dir r = opposite(g.dir);
            if (r != Dir::None && ghostPass(g, g.tr + dy(r), g.tc + dx(r))) return r;
            return Dir::None;
        }
        return opts[QRandomGenerator::global()->bounded(n)];
    }

    // Hunting: greedy Euclidean step toward target, no reverse unless forced.
    static const Dir order[4] = {Dir::Up, Dir::Left, Dir::Down, Dir::Right};
    QPoint tgt = ghostTarget(g);
    bool canRev = true;
    if (g.dir != Dir::None) {
        canRev = true;
        for (Dir d : order) {
            if (d == opposite(g.dir)) continue;
            if (ghostPass(g, g.tr + dy(d), g.tc + dx(d))) {
                canRev = false;
                break;
            }
        }
    }
    Dir best = Dir::None;
    long long bestD = -1;
    for (Dir d : order) {
        if (!canRev && d == opposite(g.dir)) continue;
        int rr, cc;
        if (!normTile(g.tr + dy(d), g.tc + dx(d), rr, cc)) continue;
        if (!ghostPass(g, rr, cc)) continue;
        long long dxr = cc - tgt.x(), dyr = rr - tgt.y();
        long long v = dxr * dxr + dyr * dyr;
        if (best == Dir::None || v < bestD) {
            bestD = v;
            best = d;
        }
    }
    return best;
}

void Game::updatePlaying(double dt) {
    if (frightT_ > 0.0) {
        frightT_ -= dt;
        if (frightT_ <= 0.0) {
            frightT_ = 0.0;
            frightChain_ = 0;
            for (auto& g : ghosts_)
                if (g.st == Ghost::St::Frightened) g.st = Ghost::St::Hunting;
        }
    } else {
        modeT_ -= dt;
        if (modeT_ <= 0.0) advanceMode();
    }

    // --- Pac-Men: move, eat dots, grab fruit ---
    for (size_t i = 0; i < players_.size(); i++) {
        Player& pl = players_[i];
        if (!pl.active()) continue;

        // Co-op: a caught player plays their death animation alone, then
        // respawns at their start tile while the rest of the game carries on.
        if (pl.deathT > 0) {
            pl.deathT -= dt;
            if (pl.deathT <= 0) {
                pl.deathT = 0;
                pl.lives--;
                if (pl.active()) {
                    spawnPlayer(int(i));
                    pl.respawnT = kRespawnGraceSecs;
                }
            }
            continue;
        }
        if (pl.respawnT > 0) pl.respawnT = std::max(0.0, pl.respawnT - dt);

        Entity& pac = pl.pac;
        pac.speed = pacSpeed_;
        advance(pac, dt,
                [this](int r, int c) { return maze_.walkablePac(r, c); },
                [this](Entity& e) { return pacDecide(e); });

        int pts = maze_.eat(pac.tr, pac.tc);
        if (pts) {
            addScore(int(i), pts);
            dotsEatenCount_++;
            sound_.playWaka();
            if (dotsEatenCount_ == 70 || dotsEatenCount_ == 170) {
                fruitActive_ = true;
                fruitT_ = 9.5;
            }
            if (pts == 50) triggerFright();
        }

        if (fruitActive_ && pac.tr == 17 && (pac.tc == 13 || pac.tc == 14)) {
            fruitActive_ = false;
            int fpts = 100 * level_;
            addScore(int(i), fpts);
            sound_.playFruit();
            popups_.push_back({13 * cfg::kTile + 4, 17 * cfg::kTile + 2, fpts, 1.0});
        }
    }

    if (fruitActive_) {
        fruitT_ -= dt;
        if (fruitT_ <= 0.0) fruitActive_ = false;
    }

    // --- Ghosts ---
    int dotsLeft = static_cast<int>(maze_.dots().size() + maze_.pellets().size());
    for (auto& g : ghosts_) {
        if (g.st == Ghost::St::House) {
            g.houseT -= float(dt);
            if (g.houseT <= 0.f) g.st = Ghost::St::Exiting;
            continue;
        }
        if (g.st == Ghost::St::Eyes) {
            g.speed = ghostSpeed_ * 2.2;
        } else if (g.st == Ghost::St::Frightened) {
            g.speed = ghostSpeed_ * 0.55;
        } else if (humanGhost(g)) {
            g.speed = pacSpeed_;  // player ghost matches Pac-Man; no Cruise Elroy boost
        } else {
            // Blinky Cruise Elroy speed scaling as dots are eaten
            if (g.id == 0) {
                if (dotsLeft <= 10) g.speed = ghostSpeed_ * 1.22;       // Elroy 2: faster than Pac-Man!
                else if (dotsLeft <= 20) g.speed = ghostSpeed_ * 1.10;  // Elroy 1: matching Pac-Man!
                else g.speed = ghostSpeed_;
            } else {
                g.speed = ghostSpeed_;
            }
        }
        advance(g, dt,
                [this, &g](int r, int c) { return ghostPass(g, r, c); },
                [this](Entity& e) { return ghostDecide(e); });
    }

    // --- Collisions: every ghost against every living Pac-Man ---
    for (auto& g : ghosts_) {
        if (g.st == Ghost::St::House || g.st == Ghost::St::Eyes) continue;
        for (size_t i = 0; i < players_.size(); i++) {
            Player& pl = players_[i];
            if (!pl.active() || pl.deathT > 0) continue;
            float ddx = g.fx() - pl.pac.fx();
            float ddy = g.fy() - pl.pac.fy();
            if (ddx * ddx + ddy * ddy >= 11.f) continue;

            if (g.st == Ghost::St::Frightened) {
                int c = std::min(frightChain_, 3);
                int ghostPts = 200 << c;
                addScore(int(i), ghostPts);
                frightChain_++;
                g.st = Ghost::St::Eyes;
                sound_.playEatGhost();
                popups_.push_back({static_cast<int>(std::round(g.fx())), static_cast<int>(std::round(g.fy())), ghostPts, 0.8});
                break;  // this ghost is now eyes, stop checking it
            }

            // Co-op: only the caught player dies; nobody else is reset.
            if (mode_ == Mode::Coop) {
                if (pl.respawnT > 0) continue;  // still in respawn grace
                pl.deathT = kDeathSecs;
                sound_.playDeath();
                continue;
            }

            // Pac-Man caught. In Versus the ghost player scores: a personal
            // catch by the red ghost is worth more than an assist from the AI team.
            if (mode_ == Mode::Versus) ghostScore_ += humanGhost(g) ? 1000 : 300;
            sound_.playDeath();
            dyingIdx_ = int(i);
            st_ = S::Dying;
            stateT_ = 1.5;
            return;
        }
    }

    // Co-op ends only once every player has used up their lives.
    if (mode_ == Mode::Coop) {
        bool anyAlive = false;
        for (const Player& pl : players_) anyAlive = anyAlive || pl.active();
        if (!anyAlive) {
            st_ = S::Over;
            return;
        }
    }

    if (maze_.empty()) {
        st_ = S::Won;
        stateT_ = 1.2;
    }
}

void Game::startNextLevel() {
    sound_.stopAll();
    level_++;
    applyTuning();
    maze_.reset(layoutForLevel(level_));
    resetActors();
    dotsEatenCount_ = 0;
    fruitActive_ = false;
    popups_.clear();
    st_ = S::Ready;
    stateT_ = 4.2;
    sound_.playIntro();
}

void Game::tick() {
    double dt = clock_.restart() / 1000.0;
    if (dt > 0.05) dt = 0.05;
    animT_ += dt;

    if (paused_) {
        update();
        return;
    }

    for (auto it = popups_.begin(); it != popups_.end();) {
        it->timer -= dt;
        if (it->timer <= 0.0) {
            it = popups_.erase(it);
        } else {
            ++it;
        }
    }

    switch (st_) {
        case S::Ready:
            stateT_ -= dt;
            if (stateT_ <= 0.0) st_ = S::Playing;
            break;
        case S::Playing:
            updatePlaying(dt);
            break;
        case S::Dying:
            stateT_ -= dt;
            if (stateT_ <= 0.0) {
                players_[dyingIdx_].lives--;
                bool anyAlive = false;
                for (const Player& pl : players_) anyAlive = anyAlive || pl.active();
                if (!anyAlive) {
                    st_ = S::Over;
                    if (mode_ == Mode::Versus) winner_ = 1;  // ghost took every life
                } else {
                    resetActors();
                    fruitActive_ = false;
                    st_ = S::Ready;
                    stateT_ = 4.2;
                    sound_.playIntro();
                }
            }
            break;
        case S::Won:
            stateT_ -= dt;
            if (stateT_ <= 0.0) {
                // Versus is a single-maze match: clearing it means Pac-Man wins.
                if (mode_ == Mode::Versus) {
                    winner_ = 0;
                    st_ = S::Over;
                    sound_.stopAll();
                    break;
                }
                // Play intermission after level 1, level 2, or every 3 levels
                if (level_ == 1 || level_ == 2 || level_ % 3 == 0) {
                    st_ = S::Intermission;
                    stateT_ = 5.6;
                    sound_.playIntermission();
                } else {
                    startNextLevel();
                }
            }
            break;
        case S::Intermission:
            stateT_ -= dt;
            if (stateT_ <= 0.0) {
                startNextLevel();
            }
            break;
        case S::Menu:
        case S::Over:
            break;
    }
    update();
}

void Game::keyPressEvent(QKeyEvent* e) {
    const int key = e->key();

    // ---------- Mode-select menu ----------
    if (st_ == S::Menu) {
        switch (key) {
            case Qt::Key_Up:
            case Qt::Key_W:
                menuSel_ = (menuSel_ + 2) % 3;
                break;
            case Qt::Key_Down:
            case Qt::Key_S:
                menuSel_ = (menuSel_ + 1) % 3;
                break;
            case Qt::Key_1: menuSel_ = 0; break;
            case Qt::Key_2: menuSel_ = 1; break;
            case Qt::Key_3: menuSel_ = 2; break;
            case Qt::Key_Return:
            case Qt::Key_Enter:
            case Qt::Key_Space:
                mode_ = static_cast<Mode>(menuSel_);
                newGame();
                break;
            case Qt::Key_M:
                sound_.toggleMute();
                break;
            case Qt::Key_Escape:
                close();
                break;
            default:
                QWidget::keyPressEvent(e);
        }
        return;
    }

    // ---------- In game ----------
    // Arrow keys always drive P1's Pac-Man.
    // WASD drives P1 in single player, P2's Pac-Man in co-op, and the red ghost in versus.
    auto wasdTarget = [this]() -> Entity* {
        if (mode_ == Mode::Coop && players_.size() > 1) return &players_[1].pac;
        if (mode_ == Mode::Versus && !ghosts_.empty()) return &ghosts_[0];
        return &players_[0].pac;
    };

    switch (key) {
        case Qt::Key_Up:    players_[0].pac.want = Dir::Up; break;
        case Qt::Key_Down:  players_[0].pac.want = Dir::Down; break;
        case Qt::Key_Left:  players_[0].pac.want = Dir::Left; break;
        case Qt::Key_Right: players_[0].pac.want = Dir::Right; break;
        case Qt::Key_W: wasdTarget()->want = Dir::Up; break;
        case Qt::Key_S: wasdTarget()->want = Dir::Down; break;
        case Qt::Key_A: wasdTarget()->want = Dir::Left; break;
        case Qt::Key_D: wasdTarget()->want = Dir::Right; break;
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (st_ == S::Intermission) {
                startNextLevel();
                return;
            }
            break;
        case Qt::Key_M:
            sound_.toggleMute();
            break;
        case Qt::Key_P:
            if (st_ == S::Playing || st_ == S::Ready) paused_ = !paused_;
            break;
        case Qt::Key_R:
            if (st_ == S::Over) newGame();
            break;
        case Qt::Key_Escape:
            // Back to the mode-select menu (ESC on the menu quits).
            sound_.stopAll();
            paused_ = false;
            st_ = S::Menu;
            break;
        default:
            QWidget::keyPressEvent(e);
            return;
    }
}

void Game::paintEvent(QPaintEvent*) {
    if (st_ == S::Menu) {
        renderMenu();
    } else if (st_ == S::Intermission) {
        renderIntermission();
    } else {
        renderScene();
    }
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);
    p.drawImage(rect(), buffer_);
}

void Game::drawCentered(int y, const QString& text, const QColor& color) {
    int w = text.size() * 4 - 1;
    pixel::drawText(buffer_, (cfg::kW - w) / 2, y, text, color);
}

void Game::drawHud() {
    const QColor white(255, 255, 255);
    auto six = [](int v) { return QString("%1").arg(v, 6, 10, QChar('0')); };

    if (mode_ == Mode::Versus) {
        pixel::drawText(buffer_, 4, 2, QStringLiteral("PAC"), QColor(255, 255, 0));
        pixel::drawText(buffer_, 4, 8, six(players_[0].score), white);
        pixel::drawText(buffer_, 113, 2, QStringLiteral("GHOST"), QColor(255, 60, 60));
        pixel::drawText(buffer_, 113, 8, six(ghostScore_), white);
    } else {
        pixel::drawText(buffer_, 4, 2, QStringLiteral("1UP"), players_[0].color);
        pixel::drawText(buffer_, 4, 8, six(players_[0].score), white);
        if (players_.size() > 1) {
            pixel::drawText(buffer_, 113, 2, QStringLiteral("2UP"), players_[1].color);
            pixel::drawText(buffer_, 113, 8, six(players_[1].score), white);
        }
    }
    pixel::drawText(buffer_, 48, 2, QStringLiteral("HIGH SCORE"), white);
    pixel::drawText(buffer_, 60, 8, six(highScore_), white);
}

void Game::renderMenu() {
    buffer_.fill(qRgb(0, 0, 0));
    const QColor white(255, 255, 255), yellow(255, 255, 0), cyan(0, 255, 255), grey(130, 130, 130);

    drawCentered(14, QStringLiteral("PAC-MAN"), yellow);
    drawCentered(22, QStringLiteral("RETRO ARCADE"), grey);

    // Little parade: Pac-Man chased by the four ghosts.
    int mCycle = static_cast<int>(animT_ * 12.0) % 4;
    int gFrame = static_cast<int>(animT_ / 0.15) % 2;
    pixel::drawPacman(buffer_, 40, 42, Dir::Left, (mCycle == 3 ? 1 : mCycle), yellow);
    for (int i = 0; i < 4; i++)
        pixel::drawGhost(buffer_, 54 + i * 12, 42, Dir::Left, gFrame, kGhostCol[i], false, false, false);

    static const char* kItems[3] = {"1 PLAYER", "2P CO-OP", "2P VERSUS"};
    for (int i = 0; i < 3; i++) {
        int y = 66 + i * 12;
        bool sel = (i == menuSel_);
        pixel::drawText(buffer_, 50, y, QString::fromLatin1(kItems[i]), sel ? yellow : white);
        if (sel) pixel::drawPacman(buffer_, 43, y + 2, Dir::Right, mCycle == 3 ? 1 : mCycle, yellow);
    }

    // Per-mode explanation
    switch (menuSel_) {
        case 0:
            drawCentered(110, QStringLiteral("CLASSIC GAME"), cyan);
            drawCentered(118, QStringLiteral("ARROWS OR WASD"), grey);
            break;
        case 1:
            drawCentered(110, QStringLiteral("TEAM UP AND CLEAR THE MAZE"), cyan);
            drawCentered(118, QStringLiteral("P1 YELLOW: ARROWS"), QColor(255, 255, 0));
            drawCentered(126, QStringLiteral("P2 GREEN: WASD"), QColor(80, 255, 80));
            break;
        case 2:
            drawCentered(110, QStringLiteral("PAC CLEARS MAZE TO WIN"), cyan);
            drawCentered(118, QStringLiteral("GHOST TAKES 3 LIVES TO WIN"), cyan);
            drawCentered(126, QStringLiteral("P1 PAC-MAN: ARROWS"), QColor(255, 255, 0));
            drawCentered(134, QStringLiteral("P2 RED GHOST: WASD"), QColor(255, 60, 60));
            break;
    }

    if (static_cast<int>(animT_ / 0.5) % 2 == 0)
        drawCentered(150, QStringLiteral("PRESS ENTER"), white);
    drawCentered(160, QString("HIGH SCORE %1").arg(highScore_), grey);
}

void Game::renderScene() {
    buffer_.fill(qRgb(0, 0, 0));

    // 1. Classic Arcade 8-Bit HUD (Top 16 logical pixels): 1UP / HIGH SCORE / 2UP or GHOST
    drawHud();

    // Mute indicator lives in the bottom strip now (top-right is used by player 2)
    if (sound_.isMuted()) {
        drawCentered(cfg::kH + cfg::kHud - 7, QStringLiteral("MUTED"), QColor(255, 60, 60));
    }

    // 2. Maze Viewport (Logical offset Y = cfg::kHud = 16)
    int yOff = cfg::kHud;

    // Maze walls: 5x5 blocky arcade tiles
    bool flash = (st_ == S::Won) && (static_cast<int>(animT_ / 0.12) % 2 == 0);
    const Theme& theme = themeForLevel(level_);
    QRgb wallOuter = flash ? qRgb(255, 255, 255) : theme.outer;
    QRgb wallInner = flash ? qRgb(160, 160, 160) : theme.inner;
    QRgb wallCore  = flash ? qRgb(255, 255, 255) : theme.outer;

    for (int r = 0; r < cfg::kRows; r++) {
        for (int c = 0; c < cfg::kCols; c++) {
            if (maze_.at(r, c) == '#') {
                int x = c * cfg::kTile;
                int y = yOff + r * cfg::kTile;
                for (int py = 0; py < 5; py++) {
                    for (int px = 0; px < 5; px++) {
                        if (py == 0 || py == 4 || px == 0 || px == 4) {
                            pixel::setPixelSafe(buffer_, x + px, y + py, wallOuter);
                        } else if (py == 1 || py == 3 || px == 1 || px == 3) {
                            pixel::setPixelSafe(buffer_, x + px, y + py, wallInner);
                        } else {
                            pixel::setPixelSafe(buffer_, x + px, y + py, wallCore);
                        }
                    }
                }
            }
        }
    }

    // Ghost house door
    QRgb doorCol = qRgb(255, 183, 174);
    for (int c = 13; c <= 14; c++) {
        for (int px = 0; px < 5; px++) {
            pixel::setPixelSafe(buffer_, c * 5 + px, yOff + 12 * 5 + 4, doorCol);
        }
    }

    // Dots: 2x2 solid pixel blocks
    QRgb dotCol = theme.dot;
    for (const QPoint& d : maze_.dots()) {
        int x = d.x() * 5 + 2;
        int y = yOff + d.y() * 5 + 2;
        pixel::setPixelSafe(buffer_, x, y, dotCol);
        pixel::setPixelSafe(buffer_, x + 1, y, dotCol);
        pixel::setPixelSafe(buffer_, x, y + 1, dotCol);
        pixel::setPixelSafe(buffer_, x + 1, y + 1, dotCol);
    }

    // Power pellets: 4x4 flashing pixel diamonds
    double ph = std::fmod(animT_, 0.4);
    if (ph < 0.25) {
        for (const QPoint& d : maze_.pellets()) {
            int x = d.x() * 5 + 1;
            int y = yOff + d.y() * 5 + 1;
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    if ((r == 0 || r == 3) && (c == 0 || c == 3)) continue;
                    pixel::setPixelSafe(buffer_, x + c, y + r, dotCol);
                }
            }
        }
    }

    // Bonus Fruit (Cherry)
    if (fruitActive_) {
        pixel::drawCherry(buffer_, 13 * 5 + 4, yOff + 17 * 5 + 2);
    }

    // Ghosts
    if (st_ != S::Dying) {
        bool frightFlash = (frightT_ > 0.0 && frightT_ < 2.0 && ph < 0.2);
        int gFrame = static_cast<int>(animT_ / 0.15) % 2;
        for (const Ghost& g : ghosts_) {
            int gx = static_cast<int>(std::round(g.fx()));
            int gy = yOff + static_cast<int>(std::round(g.fy()));
            if (g.st == Ghost::St::Eyes) {
                pixel::drawGhost(buffer_, gx, gy, g.facing, gFrame, Qt::white, false, false, true);
            } else if (g.st == Ghost::St::Frightened) {
                pixel::drawGhost(buffer_, gx, gy, g.facing, gFrame, QColor(33, 33, 222), true, frightFlash, false);
            } else {
                pixel::drawGhost(buffer_, gx, gy, g.facing, gFrame, kGhostCol[g.id], false, false, false);
            }
        }
    }

    // Pac-Men (one per living player)
    for (size_t i = 0; i < players_.size(); i++) {
        const Player& pl = players_[i];
        bool dyingNow = (st_ == S::Dying && int(i) == dyingIdx_) || pl.deathT > 0;
        if (!pl.active() && !dyingNow) continue;
        // Blink while in the post-respawn grace period
        if (!dyingNow && pl.respawnT > 0 && static_cast<int>(pl.respawnT / 0.12) % 2 == 0) continue;
        const Entity& pac = pl.pac;
        int px = static_cast<int>(std::round(pac.fx()));
        int py = yOff + static_cast<int>(std::round(pac.fy()));
        if (dyingNow) {
            double left = (pl.deathT > 0) ? pl.deathT : stateT_;
            float prog = 1.0f - static_cast<float>(left / kDeathSecs);
            pixel::drawPacmanDying(buffer_, px, py, prog, pl.color);
        } else {
            int mFrame = 1;
            if (st_ == S::Playing && pac.dir != Dir::None) {
                int cycle = static_cast<int>(animT_ * 12.0) % 4;
                mFrame = (cycle == 3) ? 1 : cycle;
            }
            pixel::drawPacman(buffer_, px, py, pac.facing, mFrame, pl.color);
        }
    }

    // Score popups (e.g. 200, 400, 800, 1600, 100)
    for (const auto& pop : popups_) {
        pixel::drawText(buffer_, pop.x - 6, yOff + pop.y - 2, QString::number(pop.points), QColor(0, 255, 255));
    }

    // Spare-lives icons: P1 bottom-left, P2 (co-op) to the left of the level indicator
    const int livesY = cfg::kH + cfg::kHud - 5;
    for (int i = 0; i < std::min(players_[0].lives - 1, 4); i++) {
        pixel::drawPacman(buffer_, 10 + i * 9, livesY, Dir::Left, 1, players_[0].color);
    }
    if (players_.size() > 1) {
        for (int i = 0; i < std::min(players_[1].lives - 1, 3); i++) {
            pixel::drawPacman(buffer_, 104 - i * 9, livesY, Dir::Right, 1, players_[1].color);
        }
    }

    // Level indicator & Cherry icon at bottom-right
    pixel::drawCherry(buffer_, cfg::kW - 10, cfg::kH + cfg::kHud - 5);
    pixel::drawText(buffer_, cfg::kW - 26, cfg::kH + cfg::kHud - 7, QString("L%1").arg(level_), QColor(255, 255, 255));

    // Center state banners
    if (st_ == S::Ready) {
        pixel::drawText(buffer_, 58, yOff + 17 * 5 - 2, QStringLiteral("READY!"), QColor(255, 255, 0));
    } else if (paused_) {
        pixel::drawText(buffer_, 58, yOff + 17 * 5 - 2, QStringLiteral("PAUSED"), QColor(0, 255, 255));
    } else if (st_ == S::Over) {
        QString title = QStringLiteral("GAME OVER");
        QColor col(255, 0, 0);
        if (mode_ == Mode::Versus) {
            title = (winner_ == 0) ? QStringLiteral("PAC-MAN WINS!") : QStringLiteral("GHOST WINS!");
            col = (winner_ == 0) ? QColor(255, 255, 0) : QColor(255, 60, 60);
        } else if (mode_ == Mode::Coop) {
            title = (players_[0].score == players_[1].score) ? QStringLiteral("GAME OVER - TIE")
                    : (players_[0].score > players_[1].score) ? QStringLiteral("GAME OVER - P1 WINS")
                                                              : QStringLiteral("GAME OVER - P2 WINS");
        }
        drawCentered(yOff + 17 * 5 - 4, title, col);
        drawCentered(yOff + 19 * 5 - 2, QStringLiteral("R AGAIN  ESC MENU"), QColor(255, 255, 255));
    }
}

void Game::renderIntermission() {
    buffer_.fill(qRgb(0, 0, 0));

    // 1. Top HUD
    drawHud();

    // 2. Stage Floor Line
    int floorY = 106;
    QRgb stageLine = qRgb(33, 33, 222);
    for (int x = 6; x < cfg::kW - 6; x++) {
        pixel::setPixelSafe(buffer_, x, floorY, stageLine);
    }

    // Act Banner & Skip Tip
    pixel::drawText(buffer_, 54, 122, QStringLiteral("ACT 1"), QColor(255, 255, 0));
    pixel::drawText(buffer_, 38, 134, QStringLiteral("INTERMISSION"), QColor(0, 255, 255));
    pixel::drawText(buffer_, 26, 152, QStringLiteral("PRESS SPACE TO SKIP"), QColor(130, 130, 130));

    // 3. Animation Timeline (5.6s total)
    float elapsed = 5.6f - static_cast<float>(stateT_);
    int standardY = 102; // Sprite bottom is at 102 + 3 = 105, directly above floor line 106

    if (elapsed < 2.6f) {
        // Part 1: Pac-Man running leftward, chased by Blinky
        float progress = elapsed / 2.5f;
        int px = static_cast<int>(155.0f - progress * 195.0f);
        int mCycle = static_cast<int>(animT_ * 14.0) % 4;
        pixel::drawPacman(buffer_, px, standardY, Dir::Left, (mCycle == 3 ? 1 : mCycle), QColor(255, 255, 0), 1);

        // Blinky chasing 24px behind
        int gx = px + 24;
        int gFrame = static_cast<int>(animT_ / 0.12) % 2;
        pixel::drawGhost(buffer_, gx, standardY, Dir::Left, gFrame, kGhostCol[0], false, false, false);
    } else {
        // Part 2: Giant Pac-Man chasing frightened Blinky rightward!
        float elapsed2 = elapsed - 2.6f;
        float progress2 = elapsed2 / 2.8f;

        // Frightened Blinky scrambling from left to right (-25 to 175)
        int gx = static_cast<int>(-25.0f + progress2 * 200.0f);
        int gFrame = static_cast<int>(animT_ / 0.10) % 2;
        pixel::drawGhost(buffer_, gx, standardY, Dir::Right, gFrame, QColor(33, 33, 222), true, false, false);

        // GIANT PAC-MAN (3x scale = 21x21 px)
        // With scale 3, radius is 9. Center at Y = 96 has bottom at 96 + 9 = 105 (aligns with floor 106!)
        int gpx = gx - 36;
        int mCycle = static_cast<int>(animT_ * 12.0) % 4;
        pixel::drawPacman(buffer_, gpx, 96, Dir::Right, (mCycle == 3 ? 1 : mCycle), QColor(255, 255, 0), 3);
    }
}
