#include "CineArt.h"
#include <QLinearGradient>
#include <QPainterPath>
#include <QPolygonF>
#include <QRadialGradient>
#include <QTransform>
#include <algorithm>
#include <cmath>

namespace hm {
namespace cineart {

namespace {
constexpr float W = 1600.0f, H = 900.0f;
constexpr float PI_F = 3.14159265f;
} // namespace

float hash(int n, int seed) {
    uint32_t x = static_cast<uint32_t>(n) * 747796405u + static_cast<uint32_t>(seed) * 2891336453u + 2891336453u;
    x = ((x >> ((x >> 28) + 4)) ^ x) * 277803737u;
    x = (x >> 22) ^ x;
    return (x & 0xFFFFFF) / 16777216.0f;
}

float vnoise(float x, int seed) {
    int i = static_cast<int>(std::floor(x));
    float f = x - i;
    f = f * f * (3.0f - 2.0f * f);
    return hash(i, seed) * (1.0f - f) + hash(i + 1, seed) * f;
}

float smooth(float x) { x = std::clamp(x, 0.0f, 1.0f); return x * x * (3.0f - 2.0f * x); }
float mixf(float a, float b, float t) { return a + (b - a) * t; }

QColor mix(const QColor& a, const QColor& b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return QColor::fromRgbF(mixf(a.redF(), b.redF(), t), mixf(a.greenF(), b.greenF(), t), mixf(a.blueF(), b.blueF(), t), mixf(a.alphaF(), b.alphaF(), t));
}

QColor alpha(const QColor& c, float a01) {
    QColor o = c;
    o.setAlphaF(std::clamp(a01, 0.0f, 1.0f));
    return o;
}

float lightning(float t, const float* strikes, int n) {
    float v = 0.0f;
    for (int i = 0; i < n; ++i) {
        float d = t - strikes[i];
        if (d < 0.0f || d > 1.3f) continue;
        // Return stroke, dark gap, weaker re-strike, long decay.
        float a = 0.0f;
        if (d < 0.07f) a = 1.0f;
        else if (d < 0.14f) a = 0.12f;
        else if (d < 0.24f) a = 0.75f;
        else a = 0.5f * std::exp(-(d - 0.24f) * 4.5f);
        v = std::max(v, a);
    }
    return v;
}

void glow(QPainter& p, QPointF c, float r, const QColor& col, float a01) {
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    QRadialGradient g(c, r);
    g.setColorAt(0.0, alpha(col, a01));
    g.setColorAt(0.35, alpha(col, a01 * 0.38f));
    g.setColorAt(1.0, alpha(col, 0.0f));
    p.setPen(Qt::NoPen);
    p.setBrush(g);
    p.drawEllipse(c, r, r);
    p.restore();
}

void sky(QPainter& p, const QColor& top, const QColor& bottom, float horizonY) {
    QLinearGradient g(0, 0, 0, horizonY);
    g.setColorAt(0.0, top);
    g.setColorAt(1.0, bottom);
    p.fillRect(QRectF(-100, -100, W + 200, H + 200), g);
}

void clouds(QPainter& p, float t, const QColor& col, float a01, float y, float h) {
    p.save();
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 7; ++i) {
        float w = 520 + hash(i, 3) * 380;
        float x = std::fmod(i * 330.0f + t * (7.0f + i * 2.5f), W + 900.0f) - 450.0f;
        float yy = y + (hash(i, 9) - 0.5f) * h;
        p.save();
        p.translate(x, yy);
        p.scale(1.0, 0.28 + hash(i, 5) * 0.12);
        QRadialGradient g(QPointF(0, 0), w * 0.5f);
        g.setColorAt(0.0, alpha(col, a01));
        g.setColorAt(1.0, alpha(col, 0.0f));
        p.setBrush(g);
        p.drawEllipse(QPointF(0, 0), w * 0.5f, w * 0.5f);
        p.restore();
    }
    p.restore();
}

void fog(QPainter& p, float t, const QColor& col, float a01, float y0, float y1) {
    p.save();
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 5; ++i) {
        float cx = std::fmod(i * 420.0f + t * (12.0f + i * 4.0f), W + 1000.0f) - 500.0f;
        float cy = y0 + (y1 - y0) * (0.2f + 0.6f * hash(i, 21));
        float rx = 520 + hash(i, 22) * 300, ry = (y1 - y0) * 0.55f;
        p.save();
        p.translate(cx, cy);
        p.scale(1.0, ry / rx);
        QRadialGradient g(QPointF(0, 0), rx);
        g.setColorAt(0.0, alpha(col, a01));
        g.setColorAt(1.0, alpha(col, 0.0f));
        p.setBrush(g);
        p.drawEllipse(QPointF(0, 0), rx, rx);
        p.restore();
    }
    p.restore();
}

void rain(QPainter& p, float t, float density, float wind, float speed, float a01) {
    p.save();
    for (int layer = 0; layer < 3; ++layer) {
        float sp = speed * (0.7f + layer * 0.45f);
        float len = 26.0f + layer * 22.0f;
        int count = static_cast<int>(density * (110 + layer * 40));
        p.setPen(QPen(QColor(175, 195, 225, static_cast<int>((40 + layer * 38) * a01)), 1.0f + layer * 0.6f, Qt::SolidLine, Qt::RoundCap));
        for (int i = 0; i < count; ++i) {
            float x = std::fmod(hash(i, layer + 40) * (W + 400.0f) + t * wind * sp * 0.5f, W + 400.0f) - 200.0f;
            float y = std::fmod(hash(i, layer + 70) * (H + 200.0f) + t * sp, H + 200.0f) - 100.0f;
            p.drawLine(QPointF(x, y), QPointF(x + wind * len * 0.045f, y + len));
        }
    }
    p.restore();
}

void embers(QPainter& p, float t, QRectF area, int count, float speed, const QColor& col) {
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        float life = std::fmod(t * speed * (0.5f + hash(i, 1)) + hash(i, 2) * 10.0f, 1.0f);
        float x = area.left() + hash(i, 3) * area.width() + std::sin(t * (1.0f + hash(i, 4) * 2.0f) + i) * 26.0f * life;
        float y = area.bottom() - life * area.height();
        float r = (1.0f + hash(i, 5) * 3.2f) * (1.0f - life * 0.7f);
        float a = std::sin(life * PI_F) * (0.45f + 0.55f * std::fabs(std::sin(t * 9.0f + i * 1.7f)));
        p.setBrush(alpha(col, a));
        p.drawEllipse(QPointF(x, y), r, r);
        if (r > 2.2f) glow(p, QPointF(x, y), r * 5.0f, col, a * 0.35f);
    }
    p.restore();
}

void flames(QPainter& p, float t, QRectF area, float intensity, int seed) {
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.setPen(Qt::NoPen);
    const float baseY = area.bottom();
    const float step = 26.0f;
    int n = static_cast<int>(area.width() / step) + 2;
    for (int layer = 0; layer < 3; ++layer) {
        float sizeK = layer == 0 ? 1.0f : (layer == 1 ? 0.68f : 0.42f);
        QColor c = layer == 0 ? QColor(220, 45, 8, 150) : (layer == 1 ? QColor(255, 125, 20, 170) : QColor(255, 225, 130, 190));
        for (int i = 0; i < n; ++i) {
            float x = area.left() + i * step + (hash(i, seed + layer) - 0.5f) * 18.0f;
            float flick = vnoise(i * 0.77f + t * (2.0f + layer * 0.9f), seed + 7);
            float h = area.height() * (0.28f + 0.72f * flick) * intensity * sizeK;
            float w = step * (1.5f + 0.5f * flick) * (0.6f + 0.4f * sizeK);
            float lean = (vnoise(t * 1.6f + i * 0.4f, seed + 13) - 0.5f) * 56.0f;
            QPainterPath f;
            f.moveTo(x - w * 0.5f, baseY);
            f.cubicTo(x - w * 0.55f, baseY - h * 0.35f, x - w * 0.12f + lean * 0.4f, baseY - h * 0.68f, x + lean, baseY - h);
            f.cubicTo(x + w * 0.12f + lean * 0.4f, baseY - h * 0.68f, x + w * 0.55f, baseY - h * 0.35f, x + w * 0.5f, baseY);
            f.closeSubpath();
            p.setBrush(c);
            p.drawPath(f);
        }
    }
    glow(p, QPointF(area.center().x(), baseY - area.height() * 0.25f), area.width() * 0.55f, QColor(255, 110, 20), 0.34f * intensity);
    p.restore();
}

void candle(QPainter& p, QPointF base, float t, float s, int seed) {
    p.save();
    p.setPen(Qt::NoPen);
    float flick = 0.82f + 0.3f * vnoise(t * 7.0f + seed * 3.1f, seed);
    QPointF top(base.x(), base.y() - 70 * s);
    QLinearGradient wax(base.x() - 8 * s, 0, base.x() + 8 * s, 0);
    wax.setColorAt(0.0, QColor(170, 160, 135));
    wax.setColorAt(0.5, QColor(235, 225, 195));
    wax.setColorAt(1.0, QColor(150, 140, 115));
    p.setBrush(wax);
    p.drawRoundedRect(QRectF(base.x() - 8 * s, top.y(), 16 * s, 70 * s), 3 * s, 3 * s);
    glow(p, QPointF(top.x(), top.y() - 14 * s), 150 * s * flick, QColor(255, 170, 70), 0.50f * flick);
    float sway = (vnoise(t * 4.0f + seed, seed + 2) - 0.5f) * 5.0f * s;
    QPainterPath f;
    f.moveTo(top.x() - 5 * s, top.y());
    f.quadTo(top.x() - 7 * s, top.y() - 14 * s * flick, top.x() + sway, top.y() - 32 * s * flick);
    f.quadTo(top.x() + 7 * s, top.y() - 14 * s * flick, top.x() + 5 * s, top.y());
    f.closeSubpath();
    p.setBrush(QColor(255, 190, 80));
    p.drawPath(f);
    p.setBrush(QColor(255, 245, 200));
    p.drawEllipse(QPointF(top.x() + sway * 0.3f, top.y() - 7 * s), 2.2 * s, 5 * s * flick);
    p.restore();
}

void rainOnGlass(QPainter& p, float t, QRectF area, int drops, float a01) {
    p.save();
    for (int i = 0; i < drops; ++i) {
        float x = area.left() + hash(i, 61) * area.width();
        bool slider = (i % 3) == 0;
        float y0 = hash(i, 62) * area.height();
        float y = slider ? std::fmod(y0 + t * (14.0f + hash(i, 63) * 36.0f), area.height() + 80.0f) - 40.0f + area.top()
                         : area.top() + y0;
        float r = 2.0f + hash(i, 64) * 5.0f;
        if (slider) {
            p.setPen(QPen(QColor(210, 225, 245, static_cast<int>(26 * a01)), r * 0.55f, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(x, y - 40 - hash(i, 65) * 60), QPointF(x, y));
        }
        p.setPen(QPen(QColor(220, 232, 250, static_cast<int>(70 * a01)), 1.0));
        p.setBrush(QColor(190, 210, 235, static_cast<int>(28 * a01)));
        p.drawEllipse(QPointF(x, y), r, r * 1.25f);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, static_cast<int>(110 * a01)));
        p.drawEllipse(QPointF(x - r * 0.3f, y - r * 0.4f), r * 0.28f, r * 0.28f);
    }
    p.restore();
}

// --------------------------------------------------------------------------
// Silhouettes
// --------------------------------------------------------------------------

void bust(QPainter& p, QPointF nb, float s, const QColor& col, float sway) {
    p.save();
    p.translate(nb.x() + sway, nb.y());
    p.scale(s, s);
    p.setPen(Qt::NoPen);
    p.setBrush(col);
    QPainterPath sh;
    sh.moveTo(-210, 260);
    sh.cubicTo(-205, 120, -190, 40, -110, 18);
    sh.cubicTo(-70, 8, -44, 0, -30, -14);
    sh.lineTo(30, -14);
    sh.cubicTo(44, 0, 70, 8, 110, 18);
    sh.cubicTo(190, 40, 205, 120, 210, 260);
    sh.closeSubpath();
    p.drawPath(sh);
    p.drawRoundedRect(QRectF(-24, -60, 48, 62), 14, 14);                  // neck
    p.drawEllipse(QPointF(0, -108), 56, 66);                              // head
    p.drawEllipse(QPointF(22, -150), 30, 28);                             // hair bun
    p.setPen(QPen(mix(col, QColor(130, 150, 190), 0.30f), 3));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(-56, -174, 112, 132), 40 * 16, 100 * 16);            // rim light on hair
    p.restore();
}

void figureBack(QPainter& p, QPointF feet, float s, const QColor& col, float sway) {
    p.save();
    p.translate(feet.x(), feet.y());
    p.scale(s, s);
    p.setPen(Qt::NoPen);
    p.setBrush(col);
    float sw = sway;
    QPainterPath coat;
    coat.moveTo(-58 + sw * 0.3f, -246);
    coat.cubicTo(-70 + sw * 0.5f, -190, -88, -110, -92, -26);
    coat.lineTo(92, -26);
    coat.cubicTo(88, -110, 70 + sw * 0.5f, -190, 58 + sw * 0.3f, -246);
    coat.cubicTo(30, -262, -30, -262, -58 + sw * 0.3f, -246);
    coat.closeSubpath();
    p.drawPath(coat);
    p.drawRect(QRectF(-34, -26, 22, 26));                                  // legs
    p.drawRect(QRectF(12, -26, 22, 26));
    p.drawRoundedRect(QRectF(-9 + sw * 0.2f, -282, 18, 36), 6, 6);         // neck
    p.drawEllipse(QPointF(sw * 0.15f, -308), 25, 29);                      // head
    p.drawEllipse(QPointF(10 + sw * 0.15f, -326), 14, 13);                 // bun
    p.setPen(QPen(mix(col, QColor(120, 140, 180), 0.35f), 2));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(-25 + sw * 0.15f, -338, 50, 58), 50 * 16, 80 * 16);
    p.restore();
}

void child(QPainter& p, QPointF feet, float s, const QColor& col, const QColor& accent, float sway) {
    p.save();
    p.translate(feet.x(), feet.y());
    p.scale(s, s);
    p.setPen(Qt::NoPen);
    p.setBrush(col);
    QPainterPath dress;
    dress.moveTo(-14 + sway * 0.2f, -112);
    dress.lineTo(-36, -34);
    dress.lineTo(36, -34);
    dress.lineTo(14 + sway * 0.2f, -112);
    dress.closeSubpath();
    p.drawPath(dress);
    p.drawRect(QRectF(-18, -34, 9, 34));
    p.drawRect(QRectF(9, -34, 9, 34));
    p.drawEllipse(QPointF(sway * 0.2f, -132), 20, 22);
    p.drawRect(QRectF(-26 + sway * 0.2f, -108, 8, 38));                    // arms
    p.drawRect(QRectF(18 + sway * 0.2f, -108, 8, 38));
    p.setBrush(accent);                                                    // hair ribbon
    QPainterPath bow;
    QPointF b(16 + sway * 0.2f, -148);
    bow.moveTo(b); bow.lineTo(b.x() + 15, b.y() - 9); bow.lineTo(b.x() + 15, b.y() + 9); bow.closeSubpath();
    bow.moveTo(b); bow.lineTo(b.x() - 4, b.y() - 12); bow.lineTo(b.x() + 6, b.y() - 12); bow.closeSubpath();
    p.drawPath(bow);
    p.restore();
}

void hand(QPainter& p, QPointF wrist, float s, float angleDeg, const QColor& col, float curl) {
    p.save();
    p.translate(wrist);
    p.rotate(angleDeg);
    p.scale(s, s);
    p.setPen(Qt::NoPen);
    p.setBrush(col);
    p.drawRoundedRect(QRectF(-26, -10, 52, 62), 14, 14);                   // palm
    p.drawRoundedRect(QRectF(-20, 40, 40, 80), 16, 16);                    // forearm
    static const float len[4] = {58, 70, 66, 52};
    for (int i = 0; i < 4; ++i) {
        float x = -23 + i * 15.5f;
        float l = len[i] * (1.0f - curl * 0.45f);
        p.save();
        p.translate(x + 6, -8);
        p.rotate(-curl * (22 + i * 6) + (i - 1.5f) * 4);
        p.drawRoundedRect(QRectF(-6.5f, -l, 13, l + 6), 6.5, 6.5);
        p.restore();
    }
    p.save();
    p.translate(-24, 22);
    p.rotate(-52 + curl * 24);
    p.drawRoundedRect(QRectF(-7, -46, 14, 54), 7, 7);                      // thumb
    p.restore();
    p.restore();
}

// --------------------------------------------------------------------------
// Set pieces
// --------------------------------------------------------------------------

namespace {
struct Win { float x, y; };
}

void manor(QPainter& p, float t, QPointF base, float s, float lit, const QColor& sil, float flash) {
    p.save();
    p.translate(base);
    p.scale(s, s);
    p.setPen(Qt::NoPen);
    p.setBrush(sil);

    auto tri = [&](float x0, float y0, float x1, float y1, float x2, float y2) {
        QPolygonF poly; poly << QPointF(x0, y0) << QPointF(x1, y1) << QPointF(x2, y2);
        p.drawPolygon(poly);
    };
    // Main hall + roof
    p.drawRect(QRectF(-330, -210, 660, 210));
    tri(-360, -208, 0, -345, 360, -208);
    // Wings
    p.drawRect(QRectF(-590, -150, 270, 150));
    tri(-610, -148, -455, -252, -300, -148);
    p.drawRect(QRectF(320, -150, 270, 150));
    tri(300, -148, 455, -252, 610, -148);
    // Central clock tower + spire
    p.drawRect(QRectF(-72, -440, 144, 240));
    tri(-96, -438, 0, -640, 96, -438);
    // Flanking turrets
    for (int side = -1; side <= 1; side += 2) {
        float x = side * 372.0f;
        p.drawRect(QRectF(x - 40, -360, 80, 220));
        tri(x - 54, -358, x, -520, x + 54, -358);
        p.drawRect(QRectF(side * 190.0f - 14, -400, 28, 120));              // chimneys
        p.drawRect(QRectF(side * 520.0f - 12, -300, 24, 100));
    }
    // Dormers
    for (int i = -2; i <= 2; ++i) tri(i * 110.0f - 30, -250 - std::fabs(i) * 14.0f, i * 110.0f, -300 - std::fabs(i) * 14.0f, i * 110.0f + 30, -250 - std::fabs(i) * 14.0f);

    // Windows (ordered pseudo-randomly so they light up one by one "as if watching")
    std::vector<Win> wins;
    for (int i = -4; i <= 4; ++i) { wins.push_back({i * 70.0f, -160}); wins.push_back({i * 70.0f, -95}); wins.push_back({i * 70.0f, -30}); }
    for (int i = -3; i <= 3; ++i) { wins.push_back({-455.0f + i * 40.0f, -95}); wins.push_back({455.0f + i * 40.0f, -95}); }
    wins.push_back({0, -400}); wins.push_back({0, -330}); wins.push_back({0, -260});
    wins.push_back({-372, -300}); wins.push_back({372, -300}); wins.push_back({-372, -230}); wins.push_back({372, -230});
    int N = static_cast<int>(wins.size());
    std::vector<int> order(N);
    for (int i = 0; i < N; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [](int a, int b) { return hash(a, 77) < hash(b, 77); });
    for (int rank = 0; rank < N; ++rank) {
        const Win& w = wins[order[rank]];
        float on = smooth(lit * (N + 4) - rank);
        QPainterPath win;
        win.moveTo(w.x - 11, w.y);
        win.lineTo(w.x - 11, w.y - 28);
        win.arcTo(QRectF(w.x - 11, w.y - 40, 22, 24), 180, -180);
        win.lineTo(w.x + 11, w.y);
        win.closeSubpath();
        if (on > 0.01f) {
            float flick = 0.8f + 0.2f * vnoise(t * 5.0f + rank * 3.0f, rank);
            QColor c = mix(QColor(255, 190, 90), QColor(255, 225, 150), hash(rank, 5));
            p.setBrush(alpha(c, on * flick));
            p.drawPath(win);
            glow(p, QPointF(w.x, w.y - 20), 60, QColor(255, 170, 70), 0.24f * on * flick);
            p.setBrush(sil);
            p.drawRect(QRectF(w.x - 0.8f, w.y - 40, 1.6f, 40));            // mullions
            p.drawRect(QRectF(w.x - 11, w.y - 18, 22, 1.6f));
        } else {
            p.setBrush(QColor(10, 12, 18, 255));
            p.drawPath(win);
        }
    }
    // A small figure stands in the central window once the house is awake.
    float watcher = smooth((lit - 0.82f) * 6.0f);
    if (watcher > 0.0f) {
        p.setBrush(alpha(QColor(8, 6, 6), watcher));
        p.drawEllipse(QPointF(0, -283), 4.5, 5.2);
        p.drawRoundedRect(QRectF(-4, -278, 8, 20), 3, 3);
    }
    if (flash > 0.0f) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(170, 190, 230, static_cast<int>(120 * flash)), 2.0));
        p.drawRect(QRectF(-330, -210, 660, 210));
        p.drawLine(QPointF(-360, -208), QPointF(0, -345));
        p.drawLine(QPointF(0, -345), QPointF(360, -208));
        p.drawLine(QPointF(-96, -438), QPointF(0, -640));
        p.drawLine(QPointF(0, -640), QPointF(96, -438));
    }
    p.restore();
}

void gate(QPainter& p, QRectF a, float open01, const QColor& iron) {
    p.save();
    for (int leaf = 0; leaf < 2; ++leaf) {
        float lw = a.width() * 0.5f;
        p.save();
        float hingeX = leaf == 0 ? a.left() : a.right();
        float dir = leaf == 0 ? 1.0f : -1.0f;
        float ang = open01 * 1.15f;                       // radians swung away from the viewer
        p.translate(hingeX, a.top());
        p.scale(std::cos(ang), 1.0f);
        p.shear(0.0, -dir * std::sin(ang) * 0.10f);
        p.scale(dir, 1.0);
        QPen pen(iron, 6.0, Qt::SolidLine, Qt::SquareCap);
        p.setPen(pen);
        p.setBrush(iron);
        float h = a.height();
        int bars = static_cast<int>(lw / 34.0f);
        for (int i = 0; i <= bars; ++i) {
            float x = i * (lw / bars);
            p.drawLine(QPointF(x, h), QPointF(x, h * 0.06f));
            QPolygonF tip; tip << QPointF(x - 9, h * 0.07f) << QPointF(x, -h * 0.02f) << QPointF(x + 9, h * 0.07f);
            p.setPen(Qt::NoPen);
            p.drawPolygon(tip);
            p.setPen(pen);
        }
        p.setPen(QPen(iron, 9.0, Qt::SolidLine, Qt::SquareCap));
        p.drawLine(QPointF(0, h * 0.18f), QPointF(lw, h * 0.18f));
        p.drawLine(QPointF(0, h * 0.82f), QPointF(lw, h * 0.82f));
        p.setPen(QPen(iron, 6.0));
        p.setBrush(Qt::NoBrush);
        for (int i = 0; i < 3; ++i) p.drawEllipse(QPointF(lw * (0.2f + 0.3f * i), h * 0.5f), 26, 26);   // scroll rings
        p.drawEllipse(QPointF(lw - 4, h * 0.5f), 34, 34);
        p.restore();
    }
    p.restore();
}

void pines(QPainter& p, float t, float y, float h, const QColor& col, float scroll, int seed) {
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(col);
    (void)t;
    float step = 78.0f;
    int n = static_cast<int>(W / step) + 4;
    float off = std::fmod(scroll, step);
    for (int i = -1; i < n; ++i) {
        int idx = i + static_cast<int>(scroll / step);
        float th = h * (0.55f + 0.45f * hash(idx, seed));
        float x = i * step - off + (hash(idx, seed + 3) - 0.5f) * 30.0f;
        for (int tier = 0; tier < 4; ++tier) {
            float ty = y - th * (0.2f * tier);
            float tw = 52.0f * (1.0f - tier * 0.2f) * (th / h);
            QPolygonF poly;
            poly << QPointF(x - tw, ty) << QPointF(x, ty - th * 0.38f) << QPointF(x + tw, ty);
            p.drawPolygon(poly);
        }
        p.drawRect(QRectF(x - 5, y, 10, 30));
    }
    p.drawRect(QRectF(-50, y + 8, W + 100, 400));
    p.restore();
}

void road(QPainter& p, float t, float speed, float hy) {
    p.save();
    p.setPen(Qt::NoPen);
    // Ground and wet asphalt
    QLinearGradient ground(0, hy, 0, H);
    ground.setColorAt(0.0, QColor(6, 8, 10));
    ground.setColorAt(1.0, QColor(3, 4, 6));
    p.fillRect(QRectF(-100, hy, W + 200, H - hy + 100), ground);
    QPolygonF asphalt;
    asphalt << QPointF(W * 0.5f - 24, hy) << QPointF(W * 0.5f + 24, hy) << QPointF(W + 560, H + 20) << QPointF(-560, H + 20);
    QLinearGradient road(0, hy, 0, H);
    road.setColorAt(0.0, QColor(26, 30, 36));
    road.setColorAt(1.0, QColor(12, 14, 18));
    p.setBrush(road);
    p.drawPolygon(asphalt);

    const int N = 16;
    float phase = std::fmod(t * speed, 1.0f);
    // Roadside pines streaming past, scaled by perspective.
    for (int side = -1; side <= 1; side += 2) {
        for (int k = N; k >= 0; --k) {
            float z = (k + 1.0f - phase) / (N + 1.0f);
            float u = std::pow(1.0f - z, 2.3f);                 // 0 far .. 1 near
            float y = hy + (H - hy) * u;
            float x = W * 0.5f + side * (60.0f + 1100.0f * u);
            float th = 40.0f + 900.0f * u;
            p.setBrush(QColor(4, 6, 8));
            for (int tier = 0; tier < 4; ++tier) {
                float ty = y - th * 0.22f * tier;
                float tw = th * 0.34f * (1.0f - tier * 0.2f);
                QPolygonF poly;
                poly << QPointF(x - tw, ty) << QPointF(x, ty - th * 0.42f) << QPointF(x + tw, ty);
                p.drawPolygon(poly);
            }
            p.drawRect(QRectF(x - th * 0.03f, y, th * 0.06f, th * 0.1f));
        }
    }
    // Centre dashes
    for (int k = 0; k < N; ++k) {
        float z0 = (k - phase) / N, z1 = (k + 0.45f - phase) / N;
        if (z1 <= 0) continue;
        z0 = std::max(z0, 0.0f);
        float u0 = std::pow(1.0f - z0, 2.3f), u1 = std::pow(1.0f - z1, 2.3f);
        float y0 = hy + (H - hy) * u0, y1 = hy + (H - hy) * u1;
        float w0 = 2.0f + 22.0f * u0, w1 = 2.0f + 22.0f * u1;
        QPolygonF d;
        d << QPointF(W * 0.5f - w0, y0) << QPointF(W * 0.5f + w0, y0) << QPointF(W * 0.5f + w1, y1) << QPointF(W * 0.5f - w1, y1);
        p.setBrush(QColor(200, 195, 160, static_cast<int>(40 + 150 * u0)));
        p.drawPolygon(d);
    }
    // Headlight beams raking the wet road and the trunks beside it
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    for (int side = -1; side <= 1; side += 2) {
        QPolygonF beam;
        float bx = W * 0.5f + side * 230.0f;
        beam << QPointF(bx - 70, H + 10) << QPointF(bx + 70, H + 10) << QPointF(W * 0.5f + side * 70.0f + 40, hy + 8) << QPointF(W * 0.5f + side * 70.0f - 40, hy + 8);
        QLinearGradient bg(0, H, 0, hy);
        bg.setColorAt(0.0, QColor(235, 230, 190, 70));
        bg.setColorAt(0.6, QColor(190, 200, 190, 28));
        bg.setColorAt(1.0, QColor(170, 190, 200, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawPolygon(beam);
    }
    p.restore();
    // Headlight pool & horizon haze
    glow(p, QPointF(W * 0.5f, hy + (H - hy) * 0.78f), 620, QColor(210, 205, 160), 0.30f);
    QLinearGradient haze(0, hy - 120, 0, hy + 90);
    haze.setColorAt(0.0, QColor(40, 50, 65, 0));
    haze.setColorAt(0.5, QColor(40, 50, 65, 120));
    haze.setColorAt(1.0, QColor(40, 50, 65, 0));
    p.fillRect(QRectF(0, hy - 120, W, 210), haze);
    p.restore();
}

void corridor(QPainter& p, float t, QPointF vp, const QColor& wall, const QColor& floor) {
    p.save();
    p.setPen(Qt::NoPen);
    float bw = 260, bh = 330;                             // back wall
    QRectF back(vp.x() - bw * 0.5f, vp.y() - bh * 0.5f, bw, bh);
    QPolygonF leftWall, rightWall, ceil, flr;
    leftWall << QPointF(-60, -80) << back.topLeft() << back.bottomLeft() << QPointF(-60, H + 80);
    rightWall << QPointF(W + 60, -80) << back.topRight() << back.bottomRight() << QPointF(W + 60, H + 80);
    ceil << QPointF(-60, -80) << QPointF(W + 60, -80) << back.topRight() << back.topLeft();
    flr << QPointF(-60, H + 80) << QPointF(W + 60, H + 80) << back.bottomRight() << back.bottomLeft();
    QLinearGradient wl(0, 0, vp.x(), 0);
    wl.setColorAt(0.0, mix(wall, Qt::black, 0.55f));
    wl.setColorAt(1.0, wall);
    p.setBrush(wl); p.drawPolygon(leftWall);
    QLinearGradient wr(W, 0, vp.x(), 0);
    wr.setColorAt(0.0, mix(wall, Qt::black, 0.55f));
    wr.setColorAt(1.0, wall);
    p.setBrush(wr); p.drawPolygon(rightWall);
    p.setBrush(mix(wall, Qt::black, 0.6f)); p.drawPolygon(ceil);
    QLinearGradient fg(0, H, 0, vp.y());
    fg.setColorAt(0.0, mix(floor, Qt::black, 0.4f));
    fg.setColorAt(1.0, floor);
    p.setBrush(fg); p.drawPolygon(flr);
    p.setBrush(mix(wall, Qt::black, 0.35f));
    p.drawRect(back);
    // Floorboard lines converge on the vanishing point.
    p.setPen(QPen(QColor(0, 0, 0, 90), 2));
    for (int i = -6; i <= 6; ++i) p.drawLine(QPointF(vp.x() + i * 220.0f, H + 80), QPointF(vp.x() + i * 14.0f, back.bottom()));
    // Portrait frames on the walls (one empty frame, the one that will matter later).
    p.setPen(Qt::NoPen);
    for (int side = -1; side <= 1; side += 2) {
        for (int k = 0; k < 4; ++k) {
            float u = 1.0f - k * 0.23f;                          // nearness
            float x = vp.x() + side * mixf(bw * 0.5f, vp.x() + 60, std::pow(u, 1.6f));
            float hh = mixf(40, 280, u * u), ww = hh * 0.62f;
            float yc = vp.y() - 20 * u;
            QPolygonF fr;
            float sk = hh * 0.14f * side;
            fr << QPointF(x, yc - hh * 0.5f - sk) << QPointF(x - side * ww, yc - hh * 0.5f) << QPointF(x - side * ww, yc + hh * 0.5f) << QPointF(x, yc + hh * 0.5f + sk);
            p.setBrush(QColor(60, 44, 24, 230));
            p.drawPolygon(fr);
            bool empty = (side == 1 && k == 1);
            p.setBrush(empty ? QColor(8, 8, 10) : QColor(22, 20, 18));
            QPolygonF in;
            float m = 0.12f;
            in << QPointF(x - side * ww * m * 0.5f, yc - hh * 0.5f * (1 - m) - sk * 0.9f) << QPointF(x - side * ww * (1 - m), yc - hh * 0.5f * (1 - m))
               << QPointF(x - side * ww * (1 - m), yc + hh * 0.5f * (1 - m)) << QPointF(x - side * ww * m * 0.5f, yc + hh * 0.5f * (1 - m) + sk * 0.9f);
            p.drawPolygon(in);
        }
    }
    (void)t;
    p.restore();
}

void musicBox(QPainter& p, QPointF c, float t, float lid, float s) {
    p.save();
    p.translate(c);
    p.scale(s, s);
    p.setPen(Qt::NoPen);
    // Body
    QLinearGradient wood(0, -10, 0, 150);
    wood.setColorAt(0.0, QColor(96, 58, 30));
    wood.setColorAt(1.0, QColor(48, 26, 12));
    p.setBrush(wood);
    p.drawRoundedRect(QRectF(-190, 0, 380, 150), 10, 10);
    p.setBrush(QColor(140, 92, 50, 160));
    p.drawRoundedRect(QRectF(-186, 2, 372, 12), 6, 6);
    // Open lid: swings up behind the body (foreshortened to a rising panel)
    float lh = 150.0f * std::sin(lid * 1.75f);
    if (lid > 0.02f) {
        QLinearGradient lw(0, -lh, 0, 0);
        lw.setColorAt(0.0, QColor(66, 38, 18));
        lw.setColorAt(1.0, QColor(110, 70, 36));
        p.setBrush(lw);
        QPolygonF lidPoly;
        lidPoly << QPointF(-176, 0) << QPointF(-160, -lh) << QPointF(160, -lh) << QPointF(176, 0);
        p.drawPolygon(lidPoly);
        p.setBrush(QColor(120, 30, 40, static_cast<int>(180 * lid)));       // red silk lining
        QPolygonF in;
        in << QPointF(-150, -6) << QPointF(-138, -lh + 10) << QPointF(138, -lh + 10) << QPointF(150, -6);
        p.drawPolygon(in);
    }
    // Mechanism: pinned cylinder + steel comb
    p.setBrush(QColor(32, 20, 12));
    p.drawRoundedRect(QRectF(-165, 16, 330, 44), 6, 6);
    QLinearGradient cyl(0, 20, 0, 54);
    cyl.setColorAt(0.0, QColor(206, 172, 98));
    cyl.setColorAt(0.5, QColor(130, 100, 50));
    cyl.setColorAt(1.0, QColor(70, 50, 24));
    p.setBrush(cyl);
    p.drawRoundedRect(QRectF(-150, 22, 300, 32), 12, 12);
    float spin = lid > 0.9f ? t * 38.0f : 0.0f;
    p.setBrush(QColor(255, 235, 170));
    for (int i = 0; i < 34; ++i) {
        float x = -144 + std::fmod(i * 9.1f + spin, 288.0f);
        float y = 26 + 22 * hash(i, 8);
        p.drawEllipse(QPointF(x, y), 1.8, 1.8);
    }
    p.setBrush(QColor(190, 195, 205));
    for (int i = 0; i < 24; ++i) p.drawRect(QRectF(-140 + i * 11.8f, 62, 5.5, 18 + (i % 6) * 5));
    p.restore();

    // Notes drifting up out of the open box
    if (lid > 0.9f) {
        p.save();
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        for (int i = 0; i < 9; ++i) {
            float life = std::fmod(t * 0.28f + hash(i, 31), 1.0f);
            QPointF q(c.x() + (hash(i, 32) - 0.5f) * 240.0f * s + std::sin(t * 1.2f + i) * 40.0f * s, c.y() - 20 * s - life * 340.0f * s);
            glow(p, q, 22 * s, QColor(255, 220, 140), std::sin(life * PI_F) * 0.7f);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 235, 180, static_cast<int>(std::sin(life * PI_F) * 255)));
            p.drawEllipse(q, 5.5 * s, 4.2 * s);
            p.drawRect(QRectF(q.x() + 4.2 * s, q.y() - 22 * s, 1.8 * s, 22 * s));
        }
        p.restore();
    }
}

void phone(QPainter& p, QPointF c, float ang, float s, bool lit, float t) {
    p.save();
    p.translate(c);
    p.rotate(ang);
    p.scale(s, s);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(14, 16, 20));
    p.drawRoundedRect(QRectF(-80, -150, 160, 300), 22, 22);
    QLinearGradient scr(0, -140, 0, 140);
    scr.setColorAt(0.0, lit ? QColor(36, 62, 96) : QColor(8, 9, 12));
    scr.setColorAt(1.0, lit ? QColor(18, 28, 46) : QColor(5, 6, 8));
    p.setBrush(scr);
    p.drawRoundedRect(QRectF(-71, -138, 142, 276), 14, 14);
    if (lit) {
        p.setBrush(QColor(230, 235, 245, 230));
        p.drawRoundedRect(QRectF(-62, -86, 124, 54), 10, 10);
        p.setBrush(QColor(120, 140, 170));
        p.drawRoundedRect(QRectF(-52, -76, 70, 7), 3, 3);
        p.setBrush(QColor(80, 90, 110));
        p.drawRoundedRect(QRectF(-52, -62, 100, 6), 3, 3);
        p.drawRoundedRect(QRectF(-52, -50, 84, 6), 3, 3);
        p.setBrush(QColor(210, 60, 60, static_cast<int>(180 + 70 * std::sin(t * 6.0f))));
        p.drawEllipse(QPointF(52, -92), 9, 9);
        p.setBrush(QColor(14, 16, 20, 255));
        p.drawRoundedRect(QRectF(-20, -145, 40, 8), 4, 4);
    }
    p.restore();
    if (lit) glow(p, c, 300 * s, QColor(90, 140, 210), 0.35f);
}

} // namespace cineart
} // namespace hm
