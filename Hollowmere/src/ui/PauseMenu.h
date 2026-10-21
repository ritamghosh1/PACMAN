#pragma once

#include <QRect>
#include <QPainter>
#include <vector>
#include <QString>

namespace hm {

enum class PauseAction {
    None,
    Resume,
    Settings,
    Save,
    LoadLastCheckpoint,
    QuitToMenu
};

struct PauseButton {
    QString text;
    PauseAction action;
    QRect rect;
    bool hovered = false;
};

class PauseOverlay {
public:
    PauseOverlay();

    void init(const QRect& bounds);
    void render(QPainter& p, const QRect& bounds);

    PauseAction onMouseMove(const QPoint& pos);
    PauseAction onMouseClick(const QPoint& pos);

private:
    std::vector<PauseButton> m_buttons;
    int m_hoveredIndex = -1;
};

} // namespace hm
