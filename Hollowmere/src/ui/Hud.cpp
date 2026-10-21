#include "Hud.h"
#include <QFont>
#include <algorithm>

namespace hm {

void Hud::draw(QPainter& p, const QRect& bounds, const World& world, float fps) {
    p.setRenderHint(QPainter::Antialiasing, true);

    int w = bounds.width();
    int h = bounds.height();
    int cx = w / 2;
    int cy = h / 2;

    const Player& player = world.player();

    // 1. Crosshair
    bool hasPrompt = !player.currentPrompt().isEmpty();
    drawCrosshair(p, cx, cy, hasPrompt);

    // 2. Prompts and Notification Toasts
    drawPrompts(p, cx, cy + 38, player.currentPrompt(), player.notificationToast(), player.toastTimer());

    // 3. Lantern Oil Gauge (Bottom Right)
    drawLanternGauge(p, w - 190, h - 55,
                     player.lantern().oil(),
                     player.lantern().isOn(),
                     player.lantern().isFocused());

    // 4. Stamina Bar (Bottom Center)
    if (player.staminaRatio() < 0.98f) {
        drawStaminaBar(p, cx - 100, h - 45, player.staminaRatio());
    }

    // 5. Objectives & Key Status (Top Left)
    drawObjectives(p, 25, 30, player.sigilCount(), player.keyFlags());

    // 6. Debug FPS (Top Right)
    p.setPen(QColor(140, 140, 150, 160));
    QFont font("Courier New", 10);
    p.setFont(font);
    p.drawText(w - 110, 25, QString("FPS: %1").arg(static_cast<int>(fps)));
}

void Hud::drawCrosshair(QPainter& p, int cx, int cy, bool hasPrompt) {
    p.setPen(Qt::NoPen);
    if (hasPrompt) {
        p.setBrush(QColor(230, 45, 45, 230)); // Red when interactive
        p.drawEllipse(QPoint(cx, cy), 4, 4);
    } else {
        p.setBrush(QColor(255, 255, 255, 120));
        p.drawEllipse(QPoint(cx, cy), 2, 2);
    }
}

void Hud::drawPrompts(QPainter& p, int cx, int cy, const QString& prompt, const QString& toast, float toastTimer) {
    if (!prompt.isEmpty()) {
        QFont font("Georgia", 13, QFont::DemiBold);
        p.setFont(font);

        QFontMetrics fm(font);
        int textW = fm.horizontalAdvance(prompt);

        QRect bgRect(cx - textW / 2 - 12, cy - 14, textW + 24, 28);
        p.setPen(QColor(180, 50, 50, 180));
        p.setBrush(QColor(15, 15, 20, 200));
        p.drawRoundedRect(bgRect, 4, 4);

        p.setPen(QColor(255, 235, 215, 250));
        p.drawText(bgRect, Qt::AlignCenter, prompt);
    }

    if (toastTimer > 0.0f && !toast.isEmpty()) {
        int alpha = static_cast<int>(std::clamp(toastTimer * 180.0f, 0.0f, 255.0f));
        QFont font("Georgia", 14, QFont::Bold);
        p.setFont(font);

        QFontMetrics fm(font);
        int textW = fm.horizontalAdvance(toast);

        QRect bgRect(cx - textW / 2 - 16, cy + 35, textW + 32, 32);
        p.setPen(QColor(160, 140, 70, alpha));
        p.setBrush(QColor(10, 10, 14, static_cast<int>(alpha * 0.85f)));
        p.drawRoundedRect(bgRect, 4, 4);

        p.setPen(QColor(255, 220, 130, alpha));
        p.drawText(bgRect, Qt::AlignCenter, toast);
    }
}

void Hud::drawLanternGauge(QPainter& p, int x, int y, float oilPercent, bool isOn, bool isFocused) {
    // Gauge background
    p.setPen(QColor(90, 80, 70, 180));
    p.setBrush(QColor(20, 18, 16, 210));
    p.drawRoundedRect(x, y, 160, 26, 4, 4);

    // Kerosene fill
    float fillW = (oilPercent / 100.0f) * 156.0f;
    QColor fillColor;
    if (oilPercent < 20.0f) {
        fillColor = QColor(220, 50, 40, 230); // Critical red
    } else if (isFocused) {
        fillColor = QColor(255, 210, 90, 240); // Bright gold when focused
    } else if (isOn) {
        fillColor = QColor(230, 160, 50, 210); // Warm amber
    } else {
        fillColor = QColor(100, 90, 80, 140);  // Dim gray when off
    }

    p.setPen(Qt::NoPen);
    p.setBrush(fillColor);
    p.drawRoundedRect(x + 2, y + 2, static_cast<int>(fillW), 22, 3, 3);

    // Text & status
    p.setPen(QColor(255, 245, 235, 240));
    QFont font("Georgia", 10, QFont::Bold);
    p.setFont(font);

    QString status = isOn ? (isFocused ? "LANTERN (FOCUS)" : "LANTERN [F]") : "LANTERN (OFF) [F]";
    p.drawText(QRect(x, y - 18, 160, 16), Qt::AlignLeft, status);
    p.drawText(QRect(x, y, 160, 26), Qt::AlignCenter, QString("%1%").arg(static_cast<int>(oilPercent)));
}

void Hud::drawStaminaBar(QPainter& p, int x, int y, float staminaRatio) {
    p.setPen(QColor(60, 70, 80, 140));
    p.setBrush(QColor(15, 20, 25, 180));
    p.drawRoundedRect(x, y, 200, 10, 2, 2);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(140, 180, 220, 200));
    p.drawRoundedRect(x + 1, y + 1, static_cast<int>(staminaRatio * 198.0f), 8, 2, 2);
}

void Hud::drawObjectives(QPainter& p, int x, int y, int sigils, int keyFlags) {
    p.setPen(QColor(240, 220, 190, 240));
    QFont titleFont("Georgia", 13, QFont::Bold);
    p.setFont(titleFont);
    p.drawText(x, y, "HOLLOWMERE");

    QFont subFont("Georgia", 11);
    p.setFont(subFont);
    p.setPen(QColor(190, 180, 170, 210));
    p.drawText(x, y + 20, QString("Memory Sigils: %1 / 4").arg(sigils));

    bool hasBrassKey = (keyFlags & (1 << 1));
    p.drawText(x, y + 38, QString("Brass Key: %1").arg(hasBrassKey ? "Acquired" : "Missing"));

    p.setPen(QColor(140, 130, 120, 160));
    QFont hintFont("Georgia", 9, QFont::StyleItalic);
    p.setFont(hintFont);
    p.drawText(x, y + 58, "WASD: Move | Shift: Sprint | C: Crouch | F: Lantern | RMB: Focus | E: Interact");
}

} // namespace hm
