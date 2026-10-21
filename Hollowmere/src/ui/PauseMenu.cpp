#include "PauseMenu.h"
#include <QFont>

namespace hm {

PauseOverlay::PauseOverlay() = default;

void PauseOverlay::init(const QRect& bounds) {
    m_buttons.clear();

    int cx = bounds.width() / 2;
    int cy = bounds.height() / 2;
    int btnW = 260;
    int btnH = 44;
    int gap = 16;
    int startY = cy - 100;

    m_buttons.push_back({"RESUME", PauseAction::Resume, QRect(cx - btnW / 2, startY, btnW, btnH), false});
    m_buttons.push_back({"SETTINGS", PauseAction::Settings, QRect(cx - btnW / 2, startY + (btnH + gap), btnW, btnH), false});
    m_buttons.push_back({"SAVE GAME", PauseAction::Save, QRect(cx - btnW / 2, startY + (btnH + gap) * 2, btnW, btnH), false});
    m_buttons.push_back({"LOAD CHECKPOINT", PauseAction::LoadLastCheckpoint, QRect(cx - btnW / 2, startY + (btnH + gap) * 3, btnW, btnH), false});
    m_buttons.push_back({"QUIT TO MAIN MENU", PauseAction::QuitToMenu, QRect(cx - btnW / 2, startY + (btnH + gap) * 4, btnW, btnH), false});
}

void PauseOverlay::render(QPainter& p, const QRect& bounds) {
    // Semi-transparent dark blur tint
    p.fillRect(bounds, QColor(8, 10, 14, 210));

    int cx = bounds.width() / 2;
    int cy = bounds.height() / 2;

    // Header
    QFont headerFont("Georgia", 32, QFont::Bold);
    headerFont.setLetterSpacing(QFont::AbsoluteSpacing, 8.0);
    p.setFont(headerFont);
    p.setPen(QColor(230, 230, 235));
    p.drawText(QRect(cx - 300, cy - 170, 600, 50), Qt::AlignCenter, "PAUSED");

    // Buttons
    QFont btnFont("Monospace", 12, QFont::Bold);
    btnFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
    p.setFont(btnFont);

    for (const auto& btn : m_buttons) {
        if (btn.hovered) {
            p.setPen(QPen(QColor(220, 180, 90, 220), 1.5f));
            p.setBrush(QColor(42, 45, 60, 230));
        } else {
            p.setPen(QPen(QColor(60, 62, 75, 140), 1.0f));
            p.setBrush(QColor(18, 20, 28, 190));
        }

        p.drawRoundedRect(btn.rect, 4, 4);

        if (btn.hovered) {
            p.setPen(QColor(255, 235, 170));
        } else {
            p.setPen(QColor(200, 200, 215));
        }

        p.drawText(btn.rect, Qt::AlignCenter, btn.text);
    }
}

PauseAction PauseOverlay::onMouseMove(const QPoint& pos) {
    m_hoveredIndex = -1;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].rect.contains(pos)) {
            m_buttons[i].hovered = true;
            m_hoveredIndex = static_cast<int>(i);
        } else {
            m_buttons[i].hovered = false;
        }
    }
    return (m_hoveredIndex >= 0) ? m_buttons[m_hoveredIndex].action : PauseAction::None;
}

PauseAction PauseOverlay::onMouseClick(const QPoint& pos) {
    for (const auto& btn : m_buttons) {
        if (btn.rect.contains(pos)) {
            return btn.action;
        }
    }
    return PauseAction::None;
}

} // namespace hm
