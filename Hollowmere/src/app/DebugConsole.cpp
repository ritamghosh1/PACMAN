#include "DebugConsole.h"
#include <QFont>

namespace hm {

DebugConsole::DebugConsole() = default;

void DebugConsole::giveAllItems(World& world) {
    auto& p = world.player();
    p.refillOil(100.0f);
    p.restoreHealth();
    // Collect everything the story needs, in order, as if the player had found it
    for (int id : world.map().questOrder) {
        for (auto& pk : world.map().pickups) {
            if (pk.id() != id || pk.isCollected()) continue;
            pk.collect();
            if (pk.type() == PickupType::Key) p.setKeyFlags(p.keyFlags() | (1 << pk.tag()));
            if (pk.type() == PickupType::MemorySigil) p.collectSigil();
        }
    }
}

void DebugConsole::unlockAllDoors(World& world) {
    for (auto& d : world.map().doors) {
        d.forceOpen();
        world.map().collision.setColliderEnabled(d.id(), false);
    }
}

void DebugConsole::summonGhosts(World& world) {
    // Test aid: drop the ghost belonging to the current wing 5 m in front of the player
    Vec3 pos = world.player().position() + world.player().forward() * 4.0f;
    int c, r;
    if (!world.map().worldToCell(pos, c, r) || world.map().grid[r][c] == '#') pos = world.player().position() - world.player().forward() * 2.4f;
    int wing = world.activeWing();
    if (wing == 0) {   // foyer: line all five up for a visual check (they are dormant here)
        Vec3 f = world.player().forward(), right(-f.z, 0, f.x);
        int i = 0;
        for (auto& g : world.ghosts()) g->resetToHome(world.player().position() + f * 6.0f + right * ((i++ - 2) * 1.8f));
        return;
    }
    for (auto& g : world.ghosts()) {
        int gw = g->type() == GhostType::Weeper ? 1 : g->type() == GhostType::HollowMan ? 2 : g->type() == GhostType::Choir ? 3 : g->type() == GhostType::Doll ? 4 : 5;
        if (gw == wing) g->resetToHome(Vec3(pos.x, 0, pos.z));
    }
}

void DebugConsole::teleportToObjective(World& world) {
    const auto& o = world.objective();
    if (!o.hasTarget) return;
    int c, r;
    Vec3 dest = o.target;
    if (world.map().worldToCell(dest, c, r)) {
        // stand on the nearest open neighbouring cell (the target itself may sit in a doorway)
        const int dc[5] = {-1, 1, 0, 0, 0}, dr[5] = {0, 0, -1, 1, 0};
        for (int k = 0; k < 5; ++k)
            if (world.map().grid[r + dr[k]][c + dc[k]] == '.') { dest = world.map().cellCenter(c + dc[k], r + dr[k]); break; }
    }
    dest.y = 0.1f;
    Vec3 to = o.target - dest;
    world.player().spawn(dest, degrees(std::atan2(to.z, to.x)));   // face the goal so E works immediately
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

    QFont dbgFont("Courier New", 9, QFont::Normal);
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
    p.drawText(rect.left() + 15, y, QString("Sigils: %1 / 4 | Keys: %2").arg(world.player().sigilsCollected()).arg(world.player().keyFlags())); y += 18;
    p.drawText(rect.left() + 15, y, QString("[F5] All items | [F6] Teleport to goal | [F7] Open all doors"));

    p.restore();
}

} // namespace hm
