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
    newGame();
    clock_.start();
    timer_->start(cfg::kTickMs);
}

void Game::newGame() {
    score_ = 0;
    lives_ = 3;
    level_ = 1;
    extraAwarded_ = false;
    paused_ = false;
    dotsEatenCount_ = 0;
    fruitActive_ = false;
    fruitT_ = 0.0;
    popups_.clear();
    applyTuning();
    maze_.reset();
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

void Game::resetActors() {
    pac_.place(13, 23, Dir::Left);
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

void Game::addScore(int n) {
    score_ += n;
    if (score_ > highScore_) {
        highScore_ = score_;
        QSettings settings("PacmanArcade", "Pacman");
        settings.setValue("highScore", highScore_);
    }
    if (!extraAwarded_ && score_ >= 10000) {
        extraAwarded_ = true;
        lives_++;
    }
}

void Game::triggerFright() {
    frightT_ = frightSecs_;
    frightChain_ = 0;
    for (auto& g : ghosts_) {
        if (g.st == Ghost::St::Hunting) {
            g.st = Ghost::St::Frightened;
            reverseMid(g);
        }
    }
}

void Game::advanceMode() {
    modeIdx_ = std::min(modeIdx_ + 1, kModeCount - 1);
    chase_ = kModes[modeIdx_].chase;
    modeT_ = kModes[modeIdx_].dur;
    for (auto& g : ghosts_)
        if (g.st == Ghost::St::Hunting) reverseMid(g);
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
    int pr = dy(pac_.facing), pc = dx(pac_.facing);
    QPoint aim(pac_.tc + pc * predictive_, pac_.tr + pr * predictive_);
    if (!chase_) return g.corner;
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

    pac_.speed = pacSpeed_;
    advance(pac_, dt,
            [this](int r, int c) { return maze_.walkablePac(r, c); },
            [this](Entity& e) { return pacDecide(e); });

    int pts = maze_.eat(pac_.tr, pac_.tc);
    if (pts) {
        addScore(pts);
        dotsEatenCount_++;
        sound_.playWaka();
        if (dotsEatenCount_ == 70 || dotsEatenCount_ == 170) {
            fruitActive_ = true;
            fruitT_ = 9.5;
        }
        if (pts == 50) triggerFright();
    }

    if (fruitActive_) {
        fruitT_ -= dt;
        if (fruitT_ <= 0.0) {
            fruitActive_ = false;
        } else if (pac_.tr == 17 && (pac_.tc == 13 || pac_.tc == 14)) {
            fruitActive_ = false;
            int fpts = 100 * level_;
            addScore(fpts);
            sound_.playFruit();
            popups_.push_back({13 * cfg::kTile + 4, 17 * cfg::kTile + 2, fpts, 1.0});
        }
    }

    for (auto& g : ghosts_) {
        if (g.st == Ghost::St::House) {
            g.houseT -= float(dt);
            if (g.houseT <= 0.f) g.st = Ghost::St::Exiting;
            continue;
        }
        if (g.st == Ghost::St::Eyes) g.speed = ghostSpeed_ * 2.2;
        else if (g.st == Ghost::St::Frightened) g.speed = ghostSpeed_ * 0.55;
        else g.speed = ghostSpeed_;
        advance(g, dt,
                [this, &g](int r, int c) { return ghostPass(g, r, c); },
                [this](Entity& e) { return ghostDecide(e); });
    }

    for (auto& g : ghosts_) {
        if (g.st == Ghost::St::House || g.st == Ghost::St::Eyes) continue;
        float ddx = g.fx() - pac_.fx();
        float ddy = g.fy() - pac_.fy();
        if (ddx * ddx + ddy * ddy < 11.f) {
            if (g.st == Ghost::St::Frightened) {
                int c = std::min(frightChain_, 3);
                int ghostPts = 200 << c;
                addScore(ghostPts);
                frightChain_++;
                g.st = Ghost::St::Eyes;
                sound_.playEatGhost();
                popups_.push_back({static_cast<int>(std::round(g.fx())), static_cast<int>(std::round(g.fy())), ghostPts, 0.8});
            } else {
                sound_.playDeath();
                st_ = S::Dying;
                stateT_ = 1.5;
                return;
            }
        }
    }

    if (maze_.empty()) {
        st_ = S::Won;
        stateT_ = 1.8;
    }
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
                lives_--;
                if (lives_ <= 0) {
                    st_ = S::Over;
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
                level_++;
                applyTuning();
                maze_.reset();
                resetActors();
                dotsEatenCount_ = 0;
                fruitActive_ = false;
                st_ = S::Ready;
                stateT_ = 4.2;
                sound_.playIntro();
            }
            break;
        case S::Over:
            break;
    }
    update();
}

void Game::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_W:
            pac_.want = Dir::Up;
            break;
        case Qt::Key_Down:
        case Qt::Key_S:
            pac_.want = Dir::Down;
            break;
        case Qt::Key_Left:
        case Qt::Key_A:
            pac_.want = Dir::Left;
            break;
        case Qt::Key_Right:
        case Qt::Key_D:
            pac_.want = Dir::Right;
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
            close();
            break;
        default:
            QWidget::keyPressEvent(e);
            return;
    }
}

void Game::paintEvent(QPaintEvent*) {
    renderScene();
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);
    p.drawImage(rect(), buffer_);
}

void Game::renderScene() {
    buffer_.fill(qRgb(0, 0, 0));

    // 1. Classic Arcade 8-Bit HUD (Top 16 logical pixels)
    // Left: "1UP" in white, Score in white
    pixel::drawText(buffer_, 4, 2, QStringLiteral("1UP"), QColor(255, 255, 255));
    pixel::drawText(buffer_, 4, 8, QString("%1").arg(score_, 6, 10, QChar('0')), QColor(255, 255, 255));

    // Center: "HIGH SCORE" in white, high score in white
    pixel::drawText(buffer_, 48, 2, QStringLiteral("HIGH SCORE"), QColor(255, 255, 255));
    pixel::drawText(buffer_, 60, 8, QString("%1").arg(highScore_, 6, 10, QChar('0')), QColor(255, 255, 255));

    // Top-right mute indicator
    if (sound_.isMuted()) {
        pixel::drawText(buffer_, 116, 2, QStringLiteral("MUTED"), QColor(255, 60, 60));
    }

    // 2. Maze Viewport (Logical offset Y = cfg::kHud = 16)
    int yOff = cfg::kHud;

    // Maze walls: 5x5 blocky arcade tiles
    bool flash = (st_ == S::Won) && (static_cast<int>(animT_ / 0.12) % 2 == 0);
    QRgb wallOuter = flash ? qRgb(255, 255, 255) : qRgb(33, 33, 222);
    QRgb wallInner = flash ? qRgb(160, 160, 160) : qRgb(10, 10, 80);
    QRgb wallCore  = flash ? qRgb(255, 255, 255) : qRgb(33, 33, 222);

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
    QRgb dotCol = qRgb(255, 183, 174);
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

    // Pac-Man
    int px = static_cast<int>(std::round(pac_.fx()));
    int py = yOff + static_cast<int>(std::round(pac_.fy()));
    if (st_ == S::Dying) {
        float prog = 1.0f - static_cast<float>(stateT_ / 1.5);
        pixel::drawPacmanDying(buffer_, px, py, prog);
    } else {
        int mFrame = 1;
        if (st_ == S::Playing && pac_.dir != Dir::None) {
            int cycle = static_cast<int>(animT_ * 12.0) % 4;
            mFrame = (cycle == 3) ? 1 : cycle;
        }
        pixel::drawPacman(buffer_, px, py, pac_.facing, mFrame, QColor(255, 255, 0));
    }

    // Score popups (e.g. 200, 400, 800, 1600, 100)
    for (const auto& pop : popups_) {
        pixel::drawText(buffer_, pop.x - 6, yOff + pop.y - 2, QString::number(pop.points), QColor(0, 255, 255));
    }

    // Lives icons at bottom-left
    for (int i = 0; i < lives_ - 1; i++) {
        pixel::drawPacman(buffer_, 10 + i * 9, cfg::kH + cfg::kHud - 5, Dir::Left, 1, QColor(255, 255, 0));
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
        pixel::drawText(buffer_, 50, yOff + 17 * 5 - 4, QStringLiteral("GAME OVER"), QColor(255, 0, 0));
        pixel::drawText(buffer_, 54, yOff + 19 * 5 - 2, QStringLiteral("PRESS R"), QColor(255, 255, 255));
    }
}
