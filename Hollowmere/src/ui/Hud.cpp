#include "Hud.h"
#include "FontCache.h"
#include <QFont>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

namespace hm {

namespace {
const QFont& serif(int px, bool bold = false, bool italic = false) { return cachedFont("Georgia", px, bold, italic); }
const QColor kGold(235, 205, 140);
} // namespace

void Hud::update(float dt) {
    m_sessionTime += dt;
    m_bannerTime += dt;
}

void Hud::showLocation(const QString& name) {
    m_bannerText = name.toUpper();
    m_bannerTime = 0.0f;
}

void Hud::draw(QPainter& p, const QRect& bounds, const World& world, float fps, const Mat4& viewProj, float time, bool showDebug) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const int w = bounds.width(), h = bounds.height();
    const Player& player = world.player();

    drawMarker(p, bounds, world, viewProj, time);
    drawCrosshair(p, w / 2, h / 2, !world.interactionPrompt().isEmpty());
    drawPrompts(p, w / 2, h / 2 + 52, world.interactionPrompt(), player.notificationToast(), player.toastTimer());
    drawObjective(p, 24, 24, world);
    drawCompass(p, w / 2, 20, world);
    if (m_mapVisible) drawMinimap(p, bounds, world, time);
    drawLanternGauge(p, w - 196, h - 52, player.lantern().oil(), player.lantern().isOn(), player.lantern().isFocused());
    drawStatus(p, 24, h - 74, world);
    drawBanner(p, bounds);
    if (m_sessionTime < 40.0f) drawControls(p, 24, h - 215);

    if (world.inSafeRoom()) {
        float a = 0.65f + 0.35f * std::sin(time * 2.0f);
        p.setFont(serif(16, false, true));
        p.setPen(QColor(255, 215, 150, static_cast<int>(210 * a)));
        p.drawText(QRect(0, h - 34, w, 24), Qt::AlignHCenter, "Candlelight. Nothing can hurt you here.");
    }
    if (world.wardenActive()) {
        p.setFont(serif(14, true));
        p.setPen(QColor(200, 90, 90, 200));
        p.drawText(QRect(0, h - 58, w, 20), Qt::AlignHCenter, "THE WARDEN IS HUNTING");
    }
    if (showDebug) {
        p.setPen(QColor(140, 140, 150, 200));
        p.setFont(cachedFont("Courier New", 13));
        p.drawText(w - 110, h - 80, QString("FPS: %1").arg(static_cast<int>(fps)));
    }
    p.restore();
}

void Hud::drawCrosshair(QPainter& p, int cx, int cy, bool hasPrompt) {
    p.setPen(Qt::NoPen);
    if (hasPrompt) {
        p.setBrush(QColor(255, 225, 150, 240));
        p.drawEllipse(QPoint(cx, cy), 5, 5);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(255, 225, 150, 160), 1.5));
        p.drawEllipse(QPoint(cx, cy), 11, 11);
    } else {
        p.setBrush(QColor(255, 255, 255, 120));
        p.drawEllipse(QPoint(cx, cy), 2, 2);
    }
}

void Hud::drawPrompts(QPainter& p, int cx, int cy, const QString& prompt, const QString& toast, float toastTimer) {
    if (!prompt.isEmpty()) {
        const QFont& font = serif(20, true);
        p.setFont(font);
        QFontMetrics fm(font);
        int textW = fm.horizontalAdvance(prompt);
        QRect bg(cx - textW / 2 - 18, cy - 18, textW + 36, 36);
        p.setPen(QPen(QColor(235, 205, 140, 190), 1.4));
        p.setBrush(QColor(12, 12, 16, 215));
        p.drawRoundedRect(bg, 6, 6);
        p.setPen(QColor(255, 240, 215));
        p.drawText(bg, Qt::AlignCenter, prompt);
    }
    if (toastTimer > 0.0f && !toast.isEmpty()) {
        int a = static_cast<int>(std::clamp(toastTimer * 200.0f, 0.0f, 255.0f));
        const QFont& font = serif(18, false, true);
        p.setFont(font);
        QFontMetrics fm(font);
        int textW = fm.horizontalAdvance(toast);
        QRect bg(cx - textW / 2 - 16, cy + 36, textW + 32, 32);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(8, 8, 12, static_cast<int>(a * 0.8f)));
        p.drawRoundedRect(bg, 5, 5);
        p.setPen(QColor(255, 225, 160, a));
        p.drawText(bg, Qt::AlignCenter, toast);
    }
}

void Hud::drawLanternGauge(QPainter& p, int x, int y, float oil, bool on, bool focused) {
    p.setFont(serif(14, true));
    p.setPen(QColor(235, 225, 210, 235));
    QString status = on ? (focused ? "LANTERN  (focused)" : "LANTERN  [F]") : "LANTERN OFF  [F]";
    p.drawText(QRect(x, y - 22, 172, 18), Qt::AlignLeft, status);
    p.setPen(QPen(QColor(110, 96, 80, 200), 1.2));
    p.setBrush(QColor(18, 16, 14, 220));
    p.drawRoundedRect(x, y, 172, 24, 4, 4);
    QColor fill = oil < 20.0f ? QColor(225, 60, 45, 235) : (focused ? QColor(255, 215, 100, 245) : (on ? QColor(235, 165, 55, 220) : QColor(110, 100, 90, 150)));
    p.setPen(Qt::NoPen);
    p.setBrush(fill);
    p.drawRoundedRect(x + 2, y + 2, static_cast<int>(oil / 100.0f * 168.0f), 20, 3, 3);
    p.setPen(QColor(255, 248, 238));
    p.setFont(serif(13, true));
    p.drawText(QRect(x, y, 172, 24), Qt::AlignCenter, QString("%1%").arg(static_cast<int>(oil)));
}

void Hud::drawStatus(QPainter& p, int x, int y, const World& world) {
    const Player& pl = world.player();
    // Health: three pips
    p.setFont(serif(13, true));
    p.setPen(QColor(210, 200, 190, 220));
    p.drawText(x, y - 6, "HEALTH");
    int pips = pl.healthState() == HealthState::Healthy ? 3 : (pl.healthState() == HealthState::Wounded ? 2 : 1);
    for (int i = 0; i < 3; ++i) {
        p.setPen(QPen(QColor(120, 40, 40), 1.2));
        p.setBrush(i < pips ? (pips == 1 ? QColor(220, 50, 50) : QColor(200, 70, 70)) : QColor(30, 14, 14));
        p.drawRoundedRect(x + 66 + i * 26, y - 20, 20, 14, 3, 3);
    }
    // Sanity bar
    p.setPen(QColor(210, 200, 190, 220));
    p.drawText(x, y + 22, "SANITY");
    p.setPen(QPen(QColor(70, 90, 120), 1.2));
    p.setBrush(QColor(12, 16, 24, 220));
    p.drawRoundedRect(x + 66, y + 8, 78, 14, 3, 3);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(110, 160, 220, 220));
    p.drawRoundedRect(x + 68, y + 10, static_cast<int>(pl.sanityNormalized() * 74.0f), 10, 2, 2);
    if (pl.staminaRatio() < 0.98f) {
        p.setPen(QColor(210, 200, 190, 220));
        p.drawText(x, y + 50, "STAMINA");
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(20, 26, 32, 220));
        p.drawRoundedRect(x + 66, y + 38, 78, 10, 2, 2);
        p.setBrush(QColor(150, 190, 225, 230));
        p.drawRoundedRect(x + 67, y + 39, static_cast<int>(pl.staminaRatio() * 76.0f), 8, 2, 2);
    }
}

void Hud::drawObjective(QPainter& p, int x, int y, const World& world) {
    const ObjectiveInfo& o = world.objective();
    int boxW = 430;
    static const QFontMetrics tfm(serif(21, true));
    static const QFontMetrics dfm(serif(15, false, true));
    QRect detailRect(x + 16, y + 56, boxW - 32, 52);
    QRect det = dfm.boundingRect(detailRect, Qt::TextWordWrap, o.detail);
    int boxH = 74 + det.height() + 34;
    p.setPen(QPen(QColor(235, 205, 140, 90), 1.2));
    p.setBrush(QColor(8, 8, 12, 175));
    p.drawRoundedRect(QRect(x, y, boxW, boxH), 8, 8);
    p.setFont(serif(12, true));
    p.setFont(cachedFont("Georgia", 12, true, false, 3.0));
    p.setPen(QColor(235, 205, 140, 220));
    p.drawText(x + 16, y + 24, "OBJECTIVE");
    p.setFont(serif(21, true));
    p.setPen(QColor(255, 244, 225));
    p.drawText(QRect(x + 16, y + 30, boxW - 32, 30), Qt::AlignVCenter, tfm.elidedText(o.title, Qt::ElideRight, boxW - 32));
    p.setFont(serif(15, false, true));
    p.setPen(QColor(205, 198, 190));
    p.drawText(detailRect, Qt::TextWordWrap, o.detail);
    // Sigil progress diamonds
    int sy = y + boxH - 22;
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 4; ++i) {
        QPointF c(x + 26 + i * 26, sy);
        QPainterPath d;
        d.moveTo(c.x(), c.y() - 9); d.lineTo(c.x() + 8, c.y()); d.lineTo(c.x(), c.y() + 9); d.lineTo(c.x() - 8, c.y()); d.closeSubpath();
        bool have = world.player().sigilCount() > i;
        p.setBrush(have ? QColor(255, 215, 110) : QColor(45, 40, 36));
        p.setPen(QPen(have ? QColor(255, 235, 170) : QColor(110, 96, 80), 1.2));
        p.drawPath(d);
    }
    p.setFont(serif(14));
    p.setPen(QColor(205, 198, 190));
    p.drawText(x + 134, sy + 5, QString("Memory sigils %1 / 4").arg(world.player().sigilCount()));
}

void Hud::drawCompass(QPainter& p, int cx, int y, const World& world) {
    const ObjectiveInfo& o = world.objective();
    if (!o.hasTarget) return;
    const Player& pl = world.player();
    Vec3 to = o.target - pl.position();
    to.y = 0;
    float dist = length(to);
    Vec3 fwd = pl.forward();
    Vec3 right(-fwd.z, 0.0f, fwd.x);
    float rel = std::atan2(dot(to, right), dot(to, fwd));
    p.save();
    p.translate(cx, y + 28);
    p.setPen(QPen(QColor(235, 205, 140, 80), 1.2));
    p.setBrush(QColor(8, 8, 12, 150));
    p.drawEllipse(QPointF(0, 0), 28, 28);
    p.rotate(degrees(rel));
    QPainterPath arrow;
    arrow.moveTo(0, -20); arrow.lineTo(12, 12); arrow.lineTo(0, 5); arrow.lineTo(-12, 12); arrow.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 215, 110));
    p.drawPath(arrow);
    p.restore();
    p.setFont(serif(15, true));
    p.setPen(QColor(240, 225, 190));
    p.drawText(QRect(cx - 60, y + 58, 120, 20), Qt::AlignCenter, QString("%1 m").arg(static_cast<int>(dist)));
}

void Hud::drawMarker(QPainter& p, const QRect& b, const World& world, const Mat4& vp, float time) {
    const ObjectiveInfo& o = world.objective();
    if (!o.hasTarget) return;
    const float* m = vp.m;
    float x = o.target.x, y = 1.4f, z = o.target.z;
    float cw = m[3] * x + m[7] * y + m[11] * z + m[15];
    if (cw < 0.3f) return;
    float nx = (m[0] * x + m[4] * y + m[8] * z + m[12]) / cw;
    float ny = (m[1] * x + m[5] * y + m[9] * z + m[13]) / cw;
    if (std::fabs(nx) > 0.95f || std::fabs(ny) > 0.95f) return;
    float sx = b.left() + (nx * 0.5f + 0.5f) * b.width();
    float sy = b.top() + (1.0f - (ny * 0.5f + 0.5f)) * b.height();
    float bob = std::sin(time * 3.0f) * 4.0f;
    float dist = distance(o.target, world.player().position());
    int a = static_cast<int>(std::clamp((dist - 2.5f) * 60.0f, 0.0f, 200.0f));
    if (a < 5) return;
    QPainterPath d;
    d.moveTo(sx, sy + bob - 14); d.lineTo(sx + 10, sy + bob); d.lineTo(sx, sy + bob + 14); d.lineTo(sx - 10, sy + bob); d.closeSubpath();
    p.setPen(QPen(QColor(255, 235, 170, a), 1.5));
    p.setBrush(QColor(255, 205, 90, a / 2));
    p.drawPath(d);
    p.setFont(serif(14, true));
    p.setPen(QColor(255, 235, 180, a));
    p.drawText(QRect(static_cast<int>(sx) - 40, static_cast<int>(sy + bob) + 18, 80, 18), Qt::AlignCenter, QString("%1 m").arg(static_cast<int>(dist)));
}

void Hud::drawMinimap(QPainter& p, const QRect& b, const World& world, float time) {
    const MapData& map = world.map();
    const int cell = 4;
    const int mw = map.gridW * cell, mh = map.gridH * cell;
    const int x0 = b.right() - mw - 24, y0 = 24;
    p.save();
    p.setPen(QPen(QColor(235, 205, 140, 90), 1.2));
    p.setBrush(QColor(6, 6, 10, 190));
    p.drawRoundedRect(QRect(x0 - 8, y0 - 8, mw + 16, mh + 36), 6, 6);
    // The static part of the map (walls, floors, doors) only changes when you explore or open a door,
    // so it is rasterised into an image once instead of ~1700 rects every frame.
    int doorSig = 0;
    for (const auto& d : map.doors) doorSig = doorSig * 3 + (d.isLocked() ? 2 : (d.isOpen() ? 1 : 0));
    if (m_mapImage.isNull() || m_mapVersion != world.exploredVersion() || m_mapDoorSig != doorSig) {
        m_mapVersion = world.exploredVersion();
        m_mapDoorSig = doorSig;
        m_mapImage = QImage(mw, mh, QImage::Format_ARGB32_Premultiplied);
        m_mapImage.fill(Qt::transparent);
        QPainter ip(&m_mapImage);
        ip.setPen(Qt::NoPen);
        const auto& ex = world.explored();
        for (int r = 0; r < map.gridH; ++r)
            for (int c = 0; c < map.gridW; ++c) {
                if (!ex[r * map.gridW + c]) continue;
                char t = map.grid[r][c];
                ip.setBrush(t == '#' ? QColor(38, 36, 40) : QColor(112, 100, 86));
                ip.drawRect(c * cell, r * cell, cell, cell);
            }
        for (const auto& d : map.doors) {
            int c, r;
            if (!map.worldToCell(d.position(), c, r) || !ex[r * map.gridW + c]) continue;
            ip.setBrush(d.isLocked() ? QColor(210, 70, 60) : (d.isOpen() ? QColor(110, 190, 120) : QColor(200, 160, 80)));
            ip.drawRect(c * cell, r * cell, cell, cell);
        }
    }
    p.drawImage(x0, y0, m_mapImage);
    p.setPen(Qt::NoPen);
    auto toMap = [&](const Vec3& v) { return QPointF(x0 + (v.x - map.origin.x) / map.cellSize * cell, y0 + (v.z - map.origin.z) / map.cellSize * cell); };
    // Objective star
    const ObjectiveInfo& o = world.objective();
    if (o.hasTarget) {
        QPointF s = toMap(o.target);
        float pulse = 4.5f + std::sin(time * 4.0f) * 1.5f;
        p.setBrush(QColor(255, 215, 90));
        p.drawEllipse(s, pulse, pulse);
        p.setBrush(QColor(255, 255, 220));
        p.drawEllipse(s, 2, 2);
    }
    // Safe rooms
    p.setBrush(QColor(255, 190, 90, 150));
    for (const auto& z : world.safeRooms().rooms())
        if (z.id != 0) p.drawEllipse(toMap(z.center), 2.5, 2.5);
    // Player arrow
    QPointF pp = toMap(world.player().position());
    Vec3 f = world.player().forward();
    p.save();
    p.translate(pp);
    p.rotate(degrees(std::atan2(f.x, -f.z)));
    QPainterPath arr;
    arr.moveTo(0, -6); arr.lineTo(4, 5); arr.lineTo(0, 2); arr.lineTo(-4, 5); arr.closeSubpath();
    p.setBrush(QColor(120, 220, 255));
    p.drawPath(arr);
    p.restore();
    p.setFont(serif(12, false, true));
    p.setPen(QColor(215, 205, 190));
    const WingInfo& wi = map.wings[std::clamp(world.activeWing(), 0, 5)];
    p.drawText(QRect(x0, y0 + mh + 2, mw, 18), Qt::AlignCenter, wi.name);
    p.restore();
}

void Hud::drawBanner(QPainter& p, const QRect& b) {
    if (m_bannerTime > 6.0f) return;
    int n = std::min(static_cast<int>(m_bannerTime / 0.06f), m_bannerText.size());
    float a = m_bannerTime < 4.5f ? 1.0f : std::max(0.0f, (6.0f - m_bannerTime) / 1.5f);
    const QFont& f = cachedFont("Georgia", 46, true, false, 10.0);
    p.save();
    p.setFont(f);
    int y = b.height() * 0.30;
    p.setPen(QColor(0, 0, 0, static_cast<int>(180 * a)));
    p.drawText(QRect(2, y + 2, b.width(), 70), Qt::AlignHCenter, m_bannerText.left(n));
    p.setPen(QColor(240, 225, 195, static_cast<int>(255 * a)));
    p.drawText(QRect(0, y, b.width(), 70), Qt::AlignHCenter, m_bannerText.left(n));
    p.setPen(QPen(QColor(190, 40, 40, static_cast<int>(200 * a)), 2));
    QFontMetrics fm(f);
    int tw = fm.horizontalAdvance(m_bannerText) / 2;
    p.drawLine(b.width() / 2 - tw, y + 74, b.width() / 2 + tw, y + 74);
    p.restore();
}

void Hud::drawControls(QPainter& p, int x, int y) {
    float a = m_sessionTime < 32.0f ? 1.0f : (40.0f - m_sessionTime) / 8.0f;
    p.setPen(QColor(210, 200, 185, static_cast<int>(200 * a)));
    p.setFont(serif(14, false, true));
    static const char* lines[] = {"W A S D  move      Mouse  look", "SHIFT  run      C  creep      F  lantern", "E / click  use      TAB  journal      M  map", "H  hum      X  close eyes      ESC  pause"};
    for (int i = 0; i < 4; ++i) p.drawText(x, y + i * 20, lines[i]);
}

} // namespace hm
