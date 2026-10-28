#pragma once

#include <QRect>
#include <QPainter>
#include <vector>
#include <QString>
#include <functional>
#include "SettingsDialog.h"

namespace hm {

enum class MenuAction {
    None,
    NewGame,
    Continue,
    Settings,
    Credits,
    Quit
};

struct MenuButton {
    QString text;
    MenuAction action;
    QRect rect;
    bool hovered = false;
};

class MenuOverlay {
public:
    MenuOverlay();

    void init(const QRect& bounds);
    void update(float dt);
    void render(QPainter& p, const QRect& bounds, bool hasSaveGame);

    MenuAction onMouseMove(const QPoint& pos);
    MenuAction onMouseClick(const QPoint& pos);

    int hoveredIndex() const { return m_hoveredIndex; }
    bool isSettingsOpen() const { return m_settingsOpen; }
    void closeSettings() { m_settingsOpen = false; }

private:
    std::vector<MenuButton> m_buttons;
    float m_animTime = 0.0f;
    int m_hoveredIndex = -1;
    bool m_settingsOpen = false;
};

} // namespace hm
