#pragma once

#include "world/World.h"
#include <QPainter>
#include <QRect>

namespace hm {

class DebugConsole {
public:
    DebugConsole();

    bool isCheatEnabled() const { return m_cheatEnabled; }
    void setCheatEnabled(bool enabled) { m_cheatEnabled = enabled; }

    bool isNoclip() const { return m_noclip; }
    void toggleNoclip() { m_noclip = !m_noclip; }

    bool areGhostsFrozen() const { return m_freezeGhosts; }
    void toggleFreezeGhosts() { m_freezeGhosts = !m_freezeGhosts; }

    bool showDebugOverlay() const { return m_showOverlay; }
    void toggleDebugOverlay() { m_showOverlay = !m_showOverlay; }

    void giveAllItems(World& world);
    void skipToWing(World& world, int wing);

    void render(QPainter& p, const QRect& bounds, const World& world, float fps);

private:
    bool m_cheatEnabled = true; // Enabled for dev and debugging verification
    bool m_noclip = false;
    bool m_freezeGhosts = false;
    bool m_showOverlay = false;
};

} // namespace hm
