#include "DebugConsole.h"
#include <QFont>

namespace hm {

DebugConsole::DebugConsole() = default;

void DebugConsole::giveAllItems(World& world) {
    auto& p = world.player();
    p.refillOil(100.0f);
    p.restoreSanity(100.0f);
    p.restoreHealth();

    // Collect all 5 Sigils
    for (int i = 0; i < 5; ++i) {
        p.collectSigil();
    }
}

void DebugConsole::skipToWing(World& world, int wing) {
    Vec3 targetPos(0.0f, 0.0f, 0.0f);
    switch (wing) {
    case 0: targetPos = Vec3(0.0f, 0.0f, 0.0f); break;   // Foyer
    case 1: targetPos = Vec3(10.0f, 0.0f, 15.0f); break; // Library
    case 2: targetPos = Vec3(-10.0f, 0.0f, 15.0f); break; // Conservatory
    case 3: targetPos = Vec3(15.0f, 0.0f, -10.0f); break; // Cellar
    case 4: targetPos = Vec3(-15.0f, 0.0f, -10.0f); break; // Nursery
    case 5: targetPos = Vec3(0.0f, 0.0f, 25.0f); break;  // Chapel
    }
    world.player().spawn(targetPos);
}

void DebugConsole::render(QPainter& p, const QRect& bounds, const World& world, float fps) {
    if (!m_showOverlay) return;

    p.save();
    int w = 320;
    int h = 180;
    QRect rect(bounds.width() - w - 20, 20, w, h);

    p.setPen(QPen(QColor(80, 180, 100), 1.0f));
    p.setBrush(QColor(10, 18, 12, 220));
    p.drawRoundedRect(rect, 6, 6);

    QFont dbgFont("Monospace", 9, QFont::Normal);
    p.setFont(dbgFont);
    p.setPen(QColor(120, 240, 140));

    const auto& pos = world.player().position();
    int y = rect.top() + 20;

    p.drawText(rect.left() + 15, y, QString("[DEBUG CONSOLE -- F3 TO HIDE]")); y += 18;
    p.drawText(rect.left() + 15, y, QString("FPS: %1 | Sim: 120 Hz").arg(static_cast<int>(fps))); y += 18;
    p.drawText(rect.left() + 15, y, QString("Player Pos: (%1, %2, %3)").arg(pos.x, 0, 'f', 1).arg(pos.y, 0, 'f', 1).arg(pos.z, 0, 'f', 1)); y += 18;
    p.drawText(rect.left() + 15, y, QString("Yaw: %1 | Fear: %2%").arg(world.player().yaw(), 0, 'f', 0).arg(static_cast<int>(world.fearLevel() * 100.0f))); y += 18;
    p.drawText(rect.left() + 15, y, QString("Ghosts Count: %1 | Frozen: %2").arg(world.ghosts().size()).arg(m_freezeGhosts ? "YES" : "NO")); y += 18;
    p.drawText(rect.left() + 15, y, QString("Noclip: %1 [F4 toggle]").arg(m_noclip ? "ENABLED" : "OFF")); y += 18;
    p.drawText(rect.left() + 15, y, QString("Sigils: %1 / 5 | Keys: %2").arg(world.player().sigilsCollected()).arg(world.player().hasBrassKey() ? "BRASS" : "NONE")); y += 18;
    p.drawText(rect.left() + 15, y, QString("[F5] Give All | [1-5] Skip Wing"));

    p.restore();
}

} // namespace hm
