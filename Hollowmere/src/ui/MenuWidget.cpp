#include "MenuWidget.h"
#include <QFont>
#include <cmath>
#include <algorithm>

namespace hm {

MenuOverlay::MenuOverlay() = default;

void MenuOverlay::init(const QRect& bounds) {
    m_buttons.clear();

    int startY = bounds.height() / 2 - 20;
    int btnW = 280;
    int btnH = 46;
    int btnX = 80; // Left aligned for sleek modern AAA aesthetic
    int gap = 16;

    m_buttons.push_back({"CONTINUE", MenuAction::Continue, QRect(btnX, startY, btnW, btnH), false});
    m_buttons.push_back({"NEW GAME", MenuAction::NewGame, QRect(btnX, startY + (btnH + gap), btnW, btnH), false});
    m_buttons.push_back({"SETTINGS", MenuAction::Settings, QRect(btnX, startY + (btnH + gap) * 2, btnW, btnH), false});
    m_buttons.push_back({"CREDITS", MenuAction::Credits, QRect(btnX, startY + (btnH + gap) * 3, btnW, btnH), false});
    m_buttons.push_back({"QUIT TO DESKTOP", MenuAction::Quit, QRect(btnX, startY + (btnH + gap) * 4, btnW, btnH), false});
}

void MenuOverlay::update(float dt) {
    m_animTime += dt;
}

void MenuOverlay::render(QPainter& p, const QRect& bounds, bool hasSaveGame) {
    // Left-side cinematic vignette gradient
    QLinearGradient grad(0, 0, bounds.width() * 0.55f, 0);
    grad.setColorAt(0.0f, QColor(6, 7, 10, 240));
    grad.setColorAt(0.7f, QColor(6, 7, 10, 180));
    grad.setColorAt(1.0f, QColor(6, 7, 10, 0));
    p.fillRect(bounds, grad);

    // Title: HOLLOWMERE
    int titleX = 80;
    int titleY = bounds.height() / 2 - 140;

    QFont titleFont("Georgia", 42, QFont::Bold);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 10.0);
    p.setFont(titleFont);

    // Subtle atmospheric title pulse
    float titlePulse = 0.85f + 0.15f * std::sin(m_animTime * 1.5f);
    p.setPen(QColor(180, 40, 40, static_cast<int>(160 * titlePulse)));
    p.drawText(titleX + 2, titleY + 2, "HOLLOWMERE");

    p.setPen(QColor(230, 230, 235));
    p.drawText(titleX, titleY, "HOLLOWMERE");

    // Subtitle
    QFont subFont("Monospace", 10, QFont::Normal);
    subFont.setLetterSpacing(QFont::AbsoluteSpacing, 3.0);
    p.setFont(subFont);
    p.setPen(QColor(160, 160, 175));
    p.drawText(titleX + 4, titleY + 36, "A PSYCHOLOGICAL HORROR EXPERIENCE");

    // Buttons
    QFont btnFont("Monospace", 13, QFont::Bold);
    btnFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
    p.setFont(btnFont);

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        auto& btn = m_buttons[i];
        bool enabled = true;
        if (btn.action == MenuAction::Continue && !hasSaveGame) {
            enabled = false;
        }

        // Button background & glow
        if (btn.hovered && enabled) {
            p.setPen(QPen(QColor(220, 180, 90, 220), 1.5f));
            p.setBrush(QColor(40, 42, 56, 180));

            // Left indicator notch
            p.fillRect(btn.rect.left() - 12, btn.rect.top() + 8, 4, btn.rect.height() - 16, QColor(220, 180, 90));
        } else {
            p.setPen(QPen(QColor(50, 52, 65, 120), 1.0f));
            p.setBrush(QColor(16, 18, 24, 140));
        }

        p.drawRoundedRect(btn.rect, 4, 4);

        // Button text
        if (!enabled) {
            p.setPen(QColor(90, 90, 100));
        } else if (btn.hovered) {
            p.setPen(QColor(255, 235, 170));
        } else {
            p.setPen(QColor(210, 210, 220));
        }

        p.drawText(btn.rect, Qt::AlignCenter, btn.text);
    }

    // Version & Tech note at bottom left
    QFont verFont("Monospace", 8, QFont::Normal);
    p.setFont(verFont);
    p.setPen(QColor(100, 105, 120));
    p.drawText(titleX, bounds.height() - 30, "v1.0.0-RELEASE | OPENGL 3.3 CORE | PROCEDURAL AUDIO & GRAPHICS");
}

MenuAction MenuOverlay::onMouseMove(const QPoint& pos) {
    m_hoveredIndex = -1;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].rect.contains(pos)) {
            m_buttons[i].hovered = true;
            m_hoveredIndex = static_cast<int>(i);
        } else {
            m_buttons[i].hovered = false;
        }
    }
    return (m_hoveredIndex >= 0) ? m_buttons[m_hoveredIndex].action : MenuAction::None;
}

MenuAction MenuOverlay::onMouseClick(const QPoint& pos) {
    for (const auto& btn : m_buttons) {
        if (btn.rect.contains(pos)) {
            return btn.action;
        }
    }
    return MenuAction::None;
}

} // namespace hm
