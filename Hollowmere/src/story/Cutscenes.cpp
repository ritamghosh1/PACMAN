#include "Cutscenes.h"
#include "CineArt.h"
#include "CutsceneKit.h"
#include "audio/AudioEngine.h"
#include "audio/CineSfx.h"
#include <QFont>
#include <QLinearGradient>
#include <QPainterPath>
#include <QRadialGradient>
#include <algorithm>
#include <cmath>

namespace hm {

using namespace cineart;
using cinesfx::Bank;
using namespace scenekit;

namespace {

// ---------------------------------------------------------------------------------------
// Shot 1 — the locked door (November 3rd, 1998)
// ---------------------------------------------------------------------------------------
void drawDoor(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(6, 5, 6));
    QPointF vp(800, 440);
    corridor(p, t, vp, QColor(52, 38, 30), QColor(58, 40, 26));

    float heat = std::clamp(smooth((t - 2.0f) / 18.0f) + (c.beat >= 3 ? 0.35f : 0.0f) + (c.beat >= 4 ? 0.3f : 0.0f), 0.0f, 1.5f);
    float flick = 0.78f + 0.22f * vnoise(t * 9.0f, 4);
    QRectF door(vp.x() - 104, vp.y() - 150, 208, 315);

    p.save();
    if (c.beat >= 2) {   // someone is pounding on the other side
        float k = c.beat >= 4 ? 1.0f : 0.5f;
        float pulse = std::max(0.0f, std::sin(t * 7.0f)) * std::pow(std::max(0.0f, std::sin(t * 1.9f)), 4.0f);
        p.translate(std::sin(t * 83.0f) * 2.2f * pulse * k, 0);
    }
    QLinearGradient body(door.topLeft(), door.bottomRight());
    body.setColorAt(0.0, QColor(74, 46, 26));
    body.setColorAt(1.0, QColor(34, 20, 12));
    p.setPen(QPen(QColor(18, 10, 6), 3));
    p.setBrush(body);
    p.drawRect(door);
    p.setBrush(QColor(0, 0, 0, 70));
    p.setPen(QPen(QColor(20, 12, 8, 190), 2));
    for (int r = 0; r < 2; ++r) for (int cidx = 0; cidx < 2; ++cidx)
        p.drawRect(QRectF(door.left() + 18 + cidx * 94, door.top() + 20 + r * 142, 78, 112));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(176, 142, 70));
    p.drawEllipse(QPointF(door.right() - 22, door.center().y() + 14), 7, 7);
    p.setBrush(QColor(22, 14, 8));
    p.drawEllipse(QPointF(door.right() - 22, door.center().y() + 32), 3, 5);   // keyhole
    // Light spilling out of the keyhole and under the door
    glow(p, QPointF(door.right() - 22, door.center().y() + 32), 40 + 60 * heat, QColor(255, 130, 40), 0.55f * heat * flick);
    QLinearGradient gap(0, door.bottom() - 9, 0, door.bottom());
    gap.setColorAt(0.0, QColor(255, 120, 30, 0));
    gap.setColorAt(1.0, QColor(255, 180, 70, static_cast<int>(std::min(1.0f, heat) * 255 * flick)));
    p.fillRect(QRectF(door.left() + 2, door.bottom() - 9, door.width() - 4, 9), gap);
    p.fillRect(QRectF(door.left() - 3, door.top() + 4, 3, door.height() - 4), QColor(255, 130, 40, static_cast<int>(std::min(1.0f, heat * 0.8f) * 160 * flick)));
    p.fillRect(QRectF(door.right(), door.top() + 4, 3, door.height() - 4), QColor(255, 130, 40, static_cast<int>(std::min(1.0f, heat * 0.8f) * 160 * flick)));
    p.restore();

    // Orange wash across the floor in front of the door
    if (heat > 0.05f) {
        p.save();
        QPolygonF spill;
        spill << QPointF(door.left(), door.bottom()) << QPointF(door.right(), door.bottom()) << QPointF(W * 0.82f, H + 40) << QPointF(W * 0.18f, H + 40);
        QLinearGradient sg(0, door.bottom(), 0, H);
        sg.setColorAt(0.0, QColor(255, 140, 50, static_cast<int>(std::min(1.0f, heat) * 150 * flick)));
        sg.setColorAt(1.0, QColor(255, 100, 30, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(sg);
        p.drawPolygon(spill);
        p.restore();
        glow(p, QPointF(vp.x(), door.bottom() + 20), 420, QColor(255, 120, 40), 0.40f * std::min(1.0f, heat) * flick);
        // smoke curling out from above the door
        p.save();
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 7; ++i) {
            float life = std::fmod(t * 0.14f + i * 0.15f, 1.0f);
            float x = door.center().x() + (hash(i, 8) - 0.5f) * 180 + std::sin(t + i) * 20 * life;
            float y = door.top() - life * 260;
            QRadialGradient g(QPointF(x, y), 70 + life * 110);
            g.setColorAt(0.0, QColor(50, 44, 42, static_cast<int>(70 * std::min(1.0f, heat) * (1 - life))));
            g.setColorAt(1.0, QColor(50, 44, 42, 0));
            p.setBrush(g);
            p.drawEllipse(QPointF(x, y), 70 + life * 110, 70 + life * 110);
        }
        p.restore();
    }
    if (heat > 0.75f) flames(p, t, QRectF(door.left() - 10, door.bottom() - 34, door.width() + 20, 40), std::min(1.0f, (heat - 0.75f) * 2.0f), 2);
    embers(p, t, QRectF(door.left() - 40, door.top(), door.width() + 80, door.height()), static_cast<int>(26 * std::min(1.0f, heat)), 0.35f, QColor(255, 170, 60));

    // Dust hanging in the dark
    p.save();
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 40; ++i) {
        float x = std::fmod(hash(i, 12) * W + t * (3 + hash(i, 13) * 6), W);
        float y = hash(i, 14) * H * 0.9f + std::sin(t * 0.4f + i) * 12;
        p.setBrush(QColor(255, 220, 180, static_cast<int>(18 + 30 * hash(i, 15))));
        p.drawEllipse(QPointF(x, y), 1.4, 1.4);
    }
    p.restore();
    if (c.beat >= 3) {   // the lights in the corridor die as the fire takes the house
        float dark = smooth((c.beatT) / 6.0f) * 0.4f;
        p.fillRect(QRectF(-100, -100, W + 200, H + 200), QColor(0, 0, 0, static_cast<int>(dark * 255)));
    }
}

// ---------------------------------------------------------------------------------------
// Shot 2 — the rain-streaked window, the buzzing phone
// ---------------------------------------------------------------------------------------
void drawWindow(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(8, 10, 14));
    QRectF win(250, 60, 1100, 520);
    // City bokeh behind the glass
    QLinearGradient night(win.topLeft(), win.bottomLeft());
    night.setColorAt(0.0, QColor(10, 16, 28));
    night.setColorAt(1.0, QColor(24, 28, 40));
    p.fillRect(win, night);
    p.save();
    p.setClipRect(win);
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 46; ++i) {
        float x = win.left() + hash(i, 21) * win.width();
        float y = win.top() + win.height() * (0.35f + 0.6f * hash(i, 22));
        float r = 14 + 34 * hash(i, 23);
        float tw = 0.7f + 0.3f * std::sin(t * (0.5f + hash(i, 24)) + i);
        int hue = i % 4;
        QColor col = hue == 0 ? QColor(255, 190, 110) : (hue == 1 ? QColor(150, 190, 255) : (hue == 2 ? QColor(255, 235, 200) : QColor(255, 90, 80)));
        QRadialGradient g(QPointF(x, y), r);
        g.setColorAt(0.0, alpha(col, 0.38f * tw));
        g.setColorAt(0.8, alpha(col, 0.20f * tw));
        g.setColorAt(1.0, alpha(col, 0.0f));
        p.setBrush(g);
        p.drawEllipse(QPointF(x, y), r, r);
    }
    p.restore();
    p.save();
    p.setClipRect(win);
    rain(p, t, 0.8f, 18.0f, 900.0f, 0.55f);
    rainOnGlass(p, t, win, 90, 1.0f);
    p.restore();

    // Window frame, mullions and sill
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(12, 12, 14));
    p.drawRect(QRectF(win.left() - 46, win.top() - 46, win.width() + 92, 46));
    p.drawRect(QRectF(win.left() - 46, win.bottom(), win.width() + 92, 40));
    p.drawRect(QRectF(win.left() - 46, win.top(), 46, win.height()));
    p.drawRect(QRectF(win.right(), win.top(), 46, win.height()));
    p.drawRect(QRectF(win.center().x() - 12, win.top(), 24, win.height()));
    p.drawRect(QRectF(win.left(), win.top() + win.height() * 0.5f - 10, win.width(), 20));
    QLinearGradient sill(0, 580, 0, 622);
    sill.setColorAt(0.0, QColor(66, 58, 50));
    sill.setColorAt(1.0, QColor(26, 22, 20));
    p.setBrush(sill);
    p.drawRect(QRectF(150, 580, 1300, 42));
    p.setBrush(QColor(6, 7, 9));
    p.drawRect(QRectF(-100, 622, W + 200, 400));

    // Nora, from behind, watching the street
    bust(p, QPointF(540, 700), 2.0f, QColor(4, 5, 8), std::sin(t * 0.7f) * 3.0f);

    // The phone on the sill: lights up and buzzes when the first line lands
    bool lit = c.beat >= 0;
    float buzz = (c.beat == 0 && c.beatT < 1.6f) ? std::sin(t * 120.0f) * 3.0f * (1.0f - c.beatT / 1.6f) : 0.0f;
    phone(p, QPointF(1130 + buzz, 556), -82 + buzz * 0.4f, 0.62f, lit, t);

    // Fluorescent tube, failing
    float fl = 0.5f + 0.5f * (vnoise(t * 14.0f, 9) > 0.35f ? 1.0f : 0.15f);
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    QLinearGradient tube(0, 0, 0, 220);
    tube.setColorAt(0.0, QColor(150, 190, 175, static_cast<int>(70 * fl)));
    tube.setColorAt(1.0, QColor(150, 190, 175, 0));
    p.fillRect(QRectF(0, 0, W, 220), tube);
    p.restore();
    // Cold teal grade
    p.fillRect(QRectF(-100, -100, W + 200, H + 200), QColor(10, 40, 46, 38));
}

// ---------------------------------------------------------------------------------------
// Shot 3 — the music box and the black-sealed envelope
// ---------------------------------------------------------------------------------------
void drawTable(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(10, 7, 6));
    // Back wall with faint wallpaper stripes
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 20; ++i) {
        p.setBrush(QColor(28 + (i % 2) * 6, 18, 16));
        p.drawRect(QRectF(i * 90.0f - 20, 0, 90, 450));
    }
    // Table
    QLinearGradient tbl(0, 450, 0, H);
    tbl.setColorAt(0.0, QColor(56, 34, 20));
    tbl.setColorAt(1.0, QColor(20, 12, 8));
    p.setBrush(tbl);
    p.drawRect(QRectF(-100, 450, W + 200, 560));
    p.setPen(QPen(QColor(0, 0, 0, 60), 2));
    for (int i = 0; i < 9; ++i) p.drawLine(QPointF(-100, 470 + i * 52.0f), QPointF(W + 100, 470 + i * 60.0f));

    // Lamp (shade only; the glow does the work) swinging a hair in a draught
    float sw = std::sin(t * 0.9f) * 6.0f;
    float flick = 0.86f + 0.14f * vnoise(t * 5.0f, 3);
    glow(p, QPointF(330 + sw, 220), 1100, QColor(255, 175, 90), 0.55f * flick);
    glow(p, QPointF(330 + sw, 470), 600, QColor(255, 190, 110), 0.30f * flick);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(20, 14, 10));
    QPolygonF shade;
    shade << QPointF(270 + sw, 80) << QPointF(390 + sw, 80) << QPointF(450 + sw, 190) << QPointF(210 + sw, 190);
    p.drawPolygon(shade);
    p.setBrush(QColor(255, 200, 120, 200));
    p.drawEllipse(QPointF(330 + sw, 196), 70, 10);
    p.setBrush(QColor(20, 14, 10));
    p.drawRect(QRectF(325 + sw * 0.4f, 190, 10, 290));

    // Mae's red hair ribbon, the only saturated thing in the room
    p.setPen(QPen(QColor(168, 24, 36), 9, Qt::SolidLine, Qt::RoundCap));
    QPainterPath rib;
    rib.moveTo(300, 560);
    rib.cubicTo(420, 520, 500, 600, 640, 560);
    p.setBrush(Qt::NoBrush);
    p.drawPath(rib);

    // Music box
    float lid = smooth((t - 2.4f) / 2.6f);
    musicBox(p, QPointF(830, 420), t, lid, 1.5f);

    // Envelope slides in from the right when it is mentioned
    if (c.beat >= 4) {
        float k = smooth(c.beatT / 1.4f);
        float ox = (1.0f - k) * 560.0f;
        p.save();
        p.translate(1280 + ox, 540);
        p.rotate(-9);
        p.setPen(QPen(QColor(90, 76, 56), 2));
        p.setBrush(QColor(222, 210, 180));
        p.drawRect(QRectF(-150, -90, 300, 180));
        p.setPen(QPen(QColor(120, 106, 84), 2));
        p.drawLine(QPointF(-150, -90), QPointF(0, 10));
        p.drawLine(QPointF(150, -90), QPointF(0, 10));
        p.setPen(QPen(QColor(70, 56, 40, 160), 2.2));
        for (int i = 0; i < 3; ++i) p.drawLine(QPointF(-100, 40 + i * 16.0f), QPointF(-100 + 120 - i * 18.0f, 40 + i * 16.0f));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(8, 8, 10));
        p.drawEllipse(QPointF(0, 8), 30, 30);
        p.setBrush(QColor(30, 30, 34));
        p.drawEllipse(QPointF(-6, 2), 8, 6);
        p.setPen(QColor(120, 120, 130));
        QFont f("Georgia");
        f.setPixelSize(30);
        f.setBold(true);
        p.setFont(f);
        p.drawText(QRectF(-30, -22, 60, 60), Qt::AlignCenter, "H");
        p.restore();
        // Nora's hand reaches for it
        hand(p, QPointF(1130 + ox * 0.6f, 860 - 130 * k), 1.45f, 24, QColor(6, 6, 8), 0.1f + 0.5f * smooth((c.beatT - 1.2f) / 1.2f));
    }
    // Dust in the lamp beam
    p.save();
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 50; ++i) {
        float x = 220 + std::fmod(hash(i, 31) * 700 + t * (4 + hash(i, 32) * 7), 700);
        float y = 160 + hash(i, 33) * 440 + std::sin(t * 0.5f + i) * 10;
        p.setBrush(QColor(255, 225, 170, static_cast<int>(20 + 55 * hash(i, 34))));
        p.drawEllipse(QPointF(x, y), 1.5, 1.5);
    }
    p.restore();
}

// ---------------------------------------------------------------------------------------
// Shot 4 — the drive: windscreen, wipers, rain, lightning, a sign that knows her name
// ---------------------------------------------------------------------------------------
void drawDriveScene(QPainter& p, const ShotContext& c) {
    float t = c.t;
    static const float strikes[] = {5.0f, 13.5f, 22.0f, 31.0f};
    float flash = lightning(t, strikes, 4);
    float hy = 410;
    sky(p, mix(QColor(8, 11, 18), QColor(120, 140, 185), flash), mix(QColor(20, 26, 36), QColor(190, 205, 235), flash), hy + 40);
    clouds(p, t, QColor(70, 82, 105), 0.35f + flash * 0.4f, 150, 160);
    road(p, t, 0.34f, hy);
    if (flash > 0.01f) p.fillRect(QRectF(-100, -100, W + 200, hy + 140), QColor(200, 215, 245, static_cast<int>(flash * 90)));

    // Roadside sign, readable only by lightning: nearest-ever-flash reveals it
    float signReveal = std::max(flash, smooth((t - 22.0f) / 1.5f) * 0.55f);
    {
        float u = smooth((t - 18.0f) / 14.0f);
        float x = mixf(900, 1180, u), y = mixf(hy - 6, hy - 60, u), s = mixf(0.25f, 1.0f, u * u);
        p.save();
        p.translate(x, y);
        p.scale(s, s);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(10, 12, 14));
        p.drawRect(QRectF(-4, 0, 8, 150));
        p.setBrush(mix(QColor(10, 12, 14), QColor(210, 205, 185), signReveal));
        p.drawRoundedRect(QRectF(-110, -90, 220, 96), 6, 6);
        QFont f("Georgia");
        f.setPixelSize(22);
        f.setBold(true);
        p.setFont(f);
        p.setPen(mix(QColor(10, 12, 14), QColor(40, 36, 30), signReveal));
        p.drawText(QRectF(-110, -86, 220, 40), Qt::AlignCenter, "ST. ALDOUS HOME");
        f.setPixelSize(17);
        p.setFont(f);
        p.drawText(QRectF(-110, -50, 220, 40), Qt::AlignCenter, "FOR CHILDREN  \xC2\xB7  2 MI");
        p.restore();
    }

    // Rain across the glass and the lights of the wipers
    rain(p, t, 0.7f, 10.0f, 1300.0f, 0.45f);
    // Windscreen frame
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(3, 4, 5));
    QPolygonF pl; pl << QPointF(-40, -40) << QPointF(150, -40) << QPointF(70, 660) << QPointF(-40, 700);
    QPolygonF pr; pr << QPointF(W + 40, -40) << QPointF(W - 150, -40) << QPointF(W - 70, 660) << QPointF(W + 40, 700);
    p.drawPolygon(pl); p.drawPolygon(pr);
    p.drawRect(QRectF(-40, -40, W + 80, 70));                    // roof line
    // Rear-view mirror
    p.setBrush(QColor(8, 9, 11));
    p.drawRoundedRect(QRectF(W * 0.5f - 130, 30, 260, 70), 12, 12);
    p.setBrush(QColor(22, 26, 32));
    p.drawRoundedRect(QRectF(W * 0.5f - 120, 38, 240, 54), 8, 8);
    // Dashboard
    QLinearGradient dash(0, 640, 0, H);
    dash.setColorAt(0.0, QColor(10, 11, 13));
    dash.setColorAt(1.0, QColor(3, 3, 4));
    p.setBrush(dash);
    QPainterPath d;
    d.moveTo(-40, 700); d.cubicTo(400, 620, 1200, 620, W + 40, 700); d.lineTo(W + 40, H + 40); d.lineTo(-40, H + 40); d.closeSubpath();
    p.drawPath(d);
    // Radio dial glow and instrument needles
    glow(p, QPointF(800, 780), 190, QColor(255, 150, 60), 0.30f);
    p.setBrush(QColor(255, 160, 70, 190));
    p.drawRoundedRect(QRectF(740, 764, 120, 18), 4, 4);
    p.setBrush(QColor(20, 10, 4));
    float needle = 740 + 60 + std::sin(t * 0.3f) * 40;
    p.drawRect(QRectF(needle, 760, 2.5, 26));
    for (int g = 0; g < 2; ++g) {
        QPointF gc(g == 0 ? 380 : 1220, 800);
        glow(p, gc, 120, QColor(120, 220, 170), 0.22f);
        p.setPen(QPen(QColor(110, 210, 160, 150), 3));
        p.setBrush(QColor(4, 8, 6));
        p.drawEllipse(gc, 62, 62);
        float a = (g == 0 ? -140 + 60 + std::sin(t * 0.4f) * 10 : -140 + 90) * 3.14159f / 180.0f;
        p.setPen(QPen(QColor(255, 120, 60), 3));
        p.drawLine(gc, gc + QPointF(std::cos(a) * 52, std::sin(a) * 52));
    }
    // Steering wheel & Nora's hands
    p.setPen(QPen(QColor(6, 7, 9), 44, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(560, 740, 480, 480), 20 * 16, 140 * 16);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(12, 12, 14));
    p.drawEllipse(QPointF(618, 840), 30, 22);
    p.drawEllipse(QPointF(982, 840), 30, 22);
    p.setBrush(QColor(170, 150, 120, 120));
    p.drawEllipse(QPointF(618, 830), 4, 3);   // wedding ring catching the dash glow

    // Wipers sweeping (cleared arcs briefly brighter)
    for (int w = 0; w < 2; ++w) {
        float ph = std::fmod(t * 0.62f + w * 0.0f, 1.0f);
        float sweep = std::sin(ph * 3.14159f);               // 0 -> 1 -> 0
        float ang = mixf(-12.0f, 78.0f, sweep) * 3.14159f / 180.0f;
        QPointF pivot(w == 0 ? 560 : 1000, 690);
        float len = 440;
        QPointF tip = pivot + QPointF(std::cos(-ang - 0.1f) * len, std::sin(-ang - 0.1f) * len);
        p.setPen(QPen(QColor(5, 6, 7), 7, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(pivot, tip);
        p.setPen(QPen(QColor(8, 9, 10), 5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(pivot + (tip - pivot) * 0.22f, tip);
    }
    rainOnGlass(p, t, QRectF(120, 70, W - 240, 620), 110, 0.9f);
    // Everything cold, slightly green
    p.fillRect(QRectF(-100, -100, W + 200, H + 200), QColor(12, 36, 40, 26));
    // Hard flash colours the whole screen
    if (flash > 0.0f) p.fillRect(QRectF(-100, -100, W + 200, H + 200), QColor(215, 225, 255, static_cast<int>(flash * 70)));
}

// ---------------------------------------------------------------------------------------
// Shot 5 — the gate and the manor, waking one window at a time
// ---------------------------------------------------------------------------------------
void drawGateScene(QPainter& p, const ShotContext& c) {
    float t = c.t;
    static const float strikes[] = {3.0f, 11.0f, 19.5f};
    float flash = lightning(t, strikes, 3);
    sky(p, mix(QColor(7, 9, 15), QColor(110, 128, 170), flash), mix(QColor(24, 30, 42), QColor(170, 185, 220), flash), 700);
    clouds(p, t, mix(QColor(48, 56, 78), QColor(150, 165, 200), flash), 0.55f, 160, 260);

    // Distant treeline either side of the manor
    pines(p, t, 720, 190, mix(QColor(5, 7, 10), QColor(32, 40, 56), flash), 0, 4);
    float lit = smooth((t - 3.0f) / 22.0f);
    manor(p, t, QPointF(800, 722), 0.96f, lit, mix(QColor(5, 6, 9), QColor(26, 30, 44), flash), flash);
    // Ground, wet gravel drive
    QLinearGradient ground(0, 700, 0, H);
    ground.setColorAt(0.0, mix(QColor(6, 8, 10), QColor(40, 48, 62), flash * 0.7f));
    ground.setColorAt(1.0, QColor(4, 5, 7));
    p.fillRect(QRectF(-100, 700, W + 200, 300), ground);
    QPolygonF drive;
    drive << QPointF(770, 706) << QPointF(830, 706) << QPointF(1250, H + 20) << QPointF(350, H + 20);
    QLinearGradient dg(0, 700, 0, H);
    dg.setColorAt(0.0, QColor(34, 38, 44, 200));
    dg.setColorAt(1.0, QColor(18, 20, 24, 255));
    p.setPen(Qt::NoPen);
    p.setBrush(dg);
    p.drawPolygon(drive);
    fog(p, t, QColor(90, 105, 130), 0.20f + flash * 0.2f, 560, 760);

    // Nora walks up the drive with her lantern, away from us
    float walk = std::min(t, 34.0f);
    QPointF feet(800 - 6 * std::sin(walk * 0.4f), 800 - walk * 3.0f);
    float s = std::max(0.40f, 0.92f - walk * 0.016f);
    figureBack(p, feet, s, QColor(4, 5, 7), std::sin(t * 2.4f) * 3.0f);
    QPointF lan(feet.x() + 46 * s, feet.y() - 122 * s + std::sin(t * 4.8f) * 3.0f);
    glow(p, lan, 190 * s, QColor(255, 175, 80), 0.62f * (0.88f + 0.12f * vnoise(t * 8.0f, 6)));
    glow(p, QPointF(feet.x(), feet.y() + 8), 260 * s, QColor(255, 180, 90), 0.20f);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 220, 140));
    p.drawEllipse(lan, 6 * s, 8 * s);

    // The gate in the foreground: wide open at first, then it closes itself
    float open = 0.86f;
    if (c.beat >= 2) open = 0.86f * (1.0f - smooth(c.beatT / 1.1f));
    else open = 0.86f - 0.06f * std::sin(t * 0.8f);
    gate(p, QRectF(-30, 380, W + 60, 520), open, mix(QColor(3, 3, 5), QColor(46, 52, 70), flash * 0.8f));
    // Stone gate pillars
    p.setPen(Qt::NoPen);
    p.setBrush(mix(QColor(12, 13, 16), QColor(60, 68, 86), flash));
    p.drawRect(QRectF(-60, 280, 150, 700));
    p.drawRect(QRectF(W - 90, 280, 150, 700));
    p.setBrush(QColor(8, 9, 11));
    p.drawRect(QRectF(-80, 262, 190, 30));
    p.drawRect(QRectF(W - 110, 262, 190, 30));
    glow(p, QPointF(-5 + 20, 250), 90, QColor(255, 170, 80), 0.18f);   // a dead lamp, smouldering

    rain(p, t, 1.0f, 22.0f, 1050.0f, 0.9f);
    if (flash > 0.0f) p.fillRect(QRectF(-100, -100, W + 200, H + 200), QColor(210, 220, 250, static_cast<int>(flash * 48)));
}

// ---------------------------------------------------------------------------------------
// Shot 6 — the title card, letters arriving like typewriter strikes with a glitch
// ---------------------------------------------------------------------------------------
void drawTitle(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(2, 2, 3));
    fog(p, t, QColor(90, 14, 18), 0.20f, 330, 620);
    embers(p, t, QRectF(0, 0, W, H), 24, 0.12f, QColor(170, 40, 30));

    const QString title = "HOLLOWMERE";
    int shown = std::clamp(static_cast<int>((t - 0.8f) / 0.22f), 0, title.size());
    QFont f("Georgia");
    f.setPixelSize(150);
    f.setBold(true);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 20.0);
    p.setFont(f);
    QFontMetrics fm(f);
    int totalW = fm.horizontalAdvance(title);
    float x0 = (W - totalW) * 0.5f;
    float glitch = (t > 3.6f && std::fmod(t, 2.3f) < 0.16f) ? 1.0f : 0.0f;
    float breathe = 0.82f + 0.18f * std::sin(t * 1.4f);
    for (int i = 0; i < shown; ++i) {
        QString ch = title.mid(i, 1);
        int xi = fm.horizontalAdvance(title.left(i));
        float born = t - (0.8f + i * 0.22f);
        float pop = born < 0.25f ? 1.0f + (0.25f - born) * 1.6f : 1.0f;     // each letter lands with a small kick
        float a = std::min(1.0f, born / 0.12f);
        p.save();
        p.translate(x0 + xi + fm.horizontalAdvance(ch) * 0.5f, 470);
        p.scale(pop, pop);
        p.translate(-fm.horizontalAdvance(ch) * 0.5f, 0);
        float dx = glitch * (hash(i, static_cast<int>(t * 30)) - 0.5f) * 28;
        p.setPen(QColor(210, 30, 40, static_cast<int>(130 * a * breathe)));
        p.drawText(QPointF(-6 + dx, 2), ch);
        p.setPen(QColor(40, 190, 220, static_cast<int>(95 * a * breathe)));
        p.drawText(QPointF(6 - dx, -2), ch);
        p.setPen(QColor(238, 236, 240, static_cast<int>(255 * a)));
        p.drawText(QPointF(0, 0), ch);
        p.restore();
    }
    if (t > 3.0f) {
        QString sub = "Some doors should stay locked.";
        int n = std::clamp(static_cast<int>((t - 3.0f) / 0.055f), 0, static_cast<int>(sub.size()));
        QFont sf("Georgia");
        sf.setPixelSize(34);
        sf.setItalic(true);
        sf.setLetterSpacing(QFont::AbsoluteSpacing, 2.5);
        p.setFont(sf);
        p.setPen(QColor(190, 175, 175, 220));
        p.drawText(QRectF(0, 520, W, 60), Qt::AlignHCenter | Qt::AlignTop, sub.left(n));
    }
    // A thin red line under the title: the crack in the door
    float crack = smooth((t - 2.4f) / 1.2f);
    p.setPen(QPen(QColor(190, 28, 36, 200), 2));
    p.drawLine(QPointF(W * 0.5f - 380 * crack, 500), QPointF(W * 0.5f + 380 * crack, 500));
}

} // namespace

void scenekit::drawDrive(QPainter& p, const ShotContext& c) { drawDriveScene(p, c); }
void scenekit::drawDoorScene(QPainter& p, const ShotContext& c) { drawDoor(p, c); }

// ===========================================================================================
// Intro assembly
// ===========================================================================================
std::unique_ptr<Cinematic> makeIntroCinematic(AudioEngine* audio) {
    const Bank& B = Bank::get();
    auto cine = std::make_unique<Cinematic>(audio);

    // 1 — November 3rd, 1998
    {
        Shot s;
        s.id = "door";
        s.draw = drawDoor;
        s.lead = 2.8f;
        s.zoom0 = 1.0f; s.zoom1 = 1.42f; s.camTime = 30.0f;
        s.fadeIn = 2.2f;
        s.onEnter = [&B](Cinematic& c) {
            bed(c, BedPad, B.padTense, 0.55f, 3.0f);
            sfx(c, B.clock, 0.7f);
        };
        s.cues = {
            {5.0f, [&B](Cinematic& c) { bed(c, BedFire, B.fire, 0.05f, 6.0f); }},
            {9.0f, [&B](Cinematic& c) { sfx(c, B.clock, 0.5f); }},
        };
        s.lines = {
            {Speaker::Narrator, U("November 3rd, 1998.")},
            {Speaker::Narrator, U("Behind a locked door, a little boy had started counting.")},
            {Speaker::Eli,      U("One… two… three…")},
            {Speaker::Eli,      U("Nora? The door is hot. Nora, please open it.")},
            {Speaker::Narrator, U("She was nine years old. She did not open it.")},
        };
        s.onLine = [&B](Cinematic& c, int i) {
            if (i == 1) bedVol(c, BedFire, 0.25f, 4.0f);
            if (i == 2) { sfx(c, B.knock, 0.8f); }
            if (i == 3) { sfx(c, B.knock, 1.0f); c.shake(0.5f); bedVol(c, BedFire, 0.6f, 2.0f); }
            if (i == 4) { sfx(c, B.thud, 1.0f); c.shake(1.0f); bedVol(c, BedFire, 0.95f, 1.5f); sfx(c, B.lock, 1.0f); }
        };
        s.outro = 1.6f;
        s.fadeOut = 1.4f;
        cine->addShot(std::move(s));
    }

    // 2 — three days missing
    {
        Shot s;
        s.id = "window";
        s.draw = drawWindow;
        s.lead = 2.0f;
        s.zoom0 = 1.0f; s.zoom1 = 1.16f; s.camTime = 24.0f;
        s.pan0 = QPointF(0, 0); s.pan1 = QPointF(-40, -10);
        s.onEnter = [&B](Cinematic& c) {
            bedStop(c, BedFire, 2.5f);
            bed(c, BedRain, B.rain, 0.55f, 2.5f);
            bed(c, BedPad, B.padLow, 0.35f, 3.0f);
            sfx(c, B.thunderB, 0.55f);
        };
        s.lines = {
            {Speaker::Narrator, U("Twenty-five years later.")},
            {Speaker::Nora,     U("Three days. Three days since the school called.")},
            {Speaker::Nora,     U("Mae never came out of class. No one saw her leave.")},
            {Speaker::Narrator, U("The police have stopped saying “missing.” They have started saying “cold.”")},
        };
        s.onLine = [&B](Cinematic& c, int i) {
            if (i == 0) sfx(c, B.buzz, 0.9f);
            if (i == 3) sfx(c, B.impact, 0.45f);
        };
        cine->addShot(std::move(s));
    }

    // 3 — the music box
    {
        Shot s;
        s.id = "musicbox";
        s.draw = drawTable;
        s.lead = 4.2f;
        s.zoom0 = 1.0f; s.zoom1 = 1.22f; s.camTime = 26.0f;
        s.pan0 = QPointF(0, 10); s.pan1 = QPointF(-30, -30);
        s.onEnter = [&B](Cinematic& c) {
            bedVol(c, BedRain, 0.22f, 2.0f);
            bed(c, BedPad, B.padLow, 0.32f, 3.0f);
        };
        s.cues = {
            {2.3f, [&B](Cinematic& c) { sfx(c, B.lock, 0.8f); }},
            {3.0f, [&B](Cinematic& c) { bed(c, BedBox, B.boxClean, 0.62f, 0.4f); }},
        };
        s.lines = {
            {Speaker::Narrator, U("It arrived this morning. No stamp. No return address.")},
            {Speaker::Nora,     U("That's my lullaby. I never taught it to anyone.")},
            {Speaker::Mae,      U("♪  hush, little light… don't say a word…  ♪")},
            {Speaker::Nora,     U("Mae?")},
            {Speaker::Narrator, U("The envelope was sealed in black wax, with an address she had spent twenty-five years forgetting.")},
        };
        s.onLine = [&B](Cinematic& c, int i) {
            if (i == 2) bed(c, BedBox, B.boxWarped, 0.62f, 0.2f);
            if (i == 3) { bedStop(c, BedBox, 0.12f); sfx(c, B.thud, 0.7f); c.shake(0.4f); }
            if (i == 4) sfx(c, B.rustle, 0.9f);
        };
        s.outro = 1.2f;
        cine->addShot(std::move(s));
    }

    // 4 — the drive
    {
        Shot s;
        s.id = "drive";
        s.draw = drawDriveScene;
        s.lead = 3.2f;
        s.zoom0 = 1.0f; s.zoom1 = 1.12f; s.camTime = 36.0f;
        s.onEnter = [&B](Cinematic& c) {
            sfx(c, B.carDoor, 0.9f);
            bedStop(c, BedPad, 2.0f);
            bedVol(c, BedRain, 0.38f, 2.0f);
            bed(c, BedCar, B.car, 0.6f, 2.5f);
            bed(c, BedPad, B.padTense, 0.3f, 6.0f);
        };
        s.cues = {
            {1.4f, [&B](Cinematic& c) { sfx(c, B.wiper, 0.5f); }},
            {3.4f, [&B](Cinematic& c) { sfx(c, B.wiper, 0.5f); }},
            {5.0f, [&B](Cinematic& c) { sfx(c, B.thunderA, 0.7f); }},
            {5.2f, [](Cinematic& c) { c.shake(0.25f); }},
            {5.3f, [&B](Cinematic& c) { sfx(c, B.wiper, 0.5f); }},
            {7.2f, [&B](Cinematic& c) { sfx(c, B.wiper, 0.5f); }},
            {9.1f, [&B](Cinematic& c) { sfx(c, B.wiper, 0.5f); }},
            {11.0f, [&B](Cinematic& c) { sfx(c, B.wiper, 0.5f); }},
            {13.5f, [&B](Cinematic& c) { sfx(c, B.thunderB, 0.8f); c.shake(0.25f); }},
            {13.0f, [&B](Cinematic& c) { sfx(c, B.wiper, 0.5f); }},
            {15.0f, [&B](Cinematic& c) { sfx(c, B.wiper, 0.5f); }},
            {22.0f, [&B](Cinematic& c) { sfx(c, B.thunderA, 0.9f); c.shake(0.3f); }},
        };
        s.lines = {
            {Speaker::Nora,   U("St. Aldous Home for Children. Closed since the fire.")},
            {Speaker::Nora,   U("Twenty-five years, and my hands still know every turn.")},
            {Speaker::Manor,  U("The road remembers you, Nora.")},
        };
        s.onLine = [&B](Cinematic& c, int i) {
            if (i == 2) { sfx(c, B.knock, 0.6f); bedVol(c, BedCar, 0.3f, 1.2f); }
        };
        s.outro = 1.4f;
        cine->addShot(std::move(s));
    }

    // 5 — the gate
    {
        Shot s;
        s.id = "gate";
        s.draw = drawGateScene;
        s.lead = 4.0f;
        s.zoom0 = 1.0f; s.zoom1 = 1.30f; s.camTime = 40.0f;
        s.pan0 = QPointF(0, 30); s.pan1 = QPointF(0, -40);
        s.onEnter = [&B](Cinematic& c) {
            sfx(c, B.carDoor, 0.8f);
            bedStop(c, BedCar, 1.0f);
            bedStop(c, BedPad, 2.0f);
            bed(c, BedWind, B.wind, 0.55f, 3.0f);
            bedVol(c, BedRain, 0.45f, 1.0f);
            sfx(c, B.gravel, 0.55f);
        };
        s.cues = {
            {3.0f, [&B](Cinematic& c) { sfx(c, B.thunderA, 0.9f); c.shake(0.3f); }},
            {11.0f, [&B](Cinematic& c) { sfx(c, B.thunderB, 0.9f); c.shake(0.3f); }},
            {19.5f, [&B](Cinematic& c) { sfx(c, B.thunderA, 1.0f); c.shake(0.35f); }},
            {6.0f, [&B](Cinematic& c) { sfx(c, B.windowOn, 0.5f, -0.4f); }},
            {9.0f, [&B](Cinematic& c) { sfx(c, B.windowOn, 0.5f, 0.3f); }},
            {13.0f, [&B](Cinematic& c) { sfx(c, B.windowOn, 0.55f, -0.1f); }},
            {16.0f, [&B](Cinematic& c) { sfx(c, B.windowOn, 0.55f, 0.5f); }},
            {19.0f, [&B](Cinematic& c) { sfx(c, B.windowOn, 0.6f, 0.0f); }},
            {2.0f, [&B](Cinematic& c) { sfx(c, B.creak, 0.55f); }},
        };
        s.lines = {
            {Speaker::Crane,   U("YOU ARE LATE, MISS HALE. THE CHILDREN HAVE BEEN WAITING.")},
            {Speaker::Nora,    U("Mae… I'm coming.")},
            {Speaker::Narrator, U("Behind her, the gate closed by itself.")},
        };
        s.onLine = [&B](Cinematic& c, int i) {
            if (i == 2) {
                sfx(c, B.clang, 1.0f);
                sfx(c, B.thud, 0.9f);
                c.shake(1.0f);
                c.flash(0.5f);
                sfx(c, B.lock, 0.8f);
            }
        };
        s.outro = 1.8f;
        s.fadeOut = 1.6f;
        cine->addShot(std::move(s));
    }

    // 6 — title
    {
        Shot s;
        s.id = "title";
        s.draw = drawTitle;
        s.autoDuration = 7.5f;
        s.zoom0 = 1.0f; s.zoom1 = 1.05f; s.camTime = 9.0f;
        s.fadeIn = 1.4f;
        s.fadeOut = 2.0f;
        s.onEnter = [&B](Cinematic& c) {
            bedStop(c, BedRain, 2.0f);
            bedStop(c, BedWind, 2.0f);
            bed(c, BedPad, B.padTense, 0.5f, 1.0f);
            sfx(c, B.riser, 0.6f);
        };
        s.cues = {{3.2f, [&B](Cinematic& c) { sfx(c, B.impact, 1.0f); c.shake(0.8f); c.flash(0.25f); }}};
        cine->addShot(std::move(s));
    }
    return cine;
}

} // namespace hm
