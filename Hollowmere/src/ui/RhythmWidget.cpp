#include "RhythmWidget.h"
#include "FontCache.h"
#include <QFont>
#include <cmath>

namespace hm {

RhythmOverlay::RhythmOverlay() = default;

void RhythmOverlay::render(QPainter& p, const QRect& bounds, const MercySystem& mercy) {
    if (!mercy.isActive()) return;

    int cx = bounds.width() / 2;
    int cy = bounds.height() / 2 - 80;

    int barW = 280;
    int barH = 14;

    // Background panel
    p.setPen(QPen(QColor(180, 150, 100, 180), 1.0f));
    p.setBrush(QColor(10, 12, 18, 210));
    p.drawRoundedRect(cx - barW / 2 - 80, cy - 40, barW + 160, 80, 8, 8);

    p.setFont(cachedFont("Georgia", 14, true));
    p.setPen(QColor(230, 210, 170));
    p.drawText(QRect(cx - barW / 2 - 80, cy - 32, barW + 160, 25), Qt::AlignCenter, "HUM TO ADA   -   click or SPACE in the green");

    // Bar background
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(40, 42, 50));
    p.drawRoundedRect(cx - barW / 2, cy + 2, barW, barH, 4, 4);

    // Target Sweetspot (green)
    int targetX = cx - barW / 2 + static_cast<int>(barW * 0.40f);
    int targetW = static_cast<int>(barW * 0.20f);
    p.setBrush(QColor(60, 180, 80, 180));
    p.drawRoundedRect(targetX, cy + 2, targetW, barH, 4, 4);

    // Moving indicator (yellow/white)
    int indX = cx - barW / 2 + static_cast<int>(barW * mercy.needlePos());
    p.setBrush(QColor(255, 230, 140));
    p.drawRoundedRect(indX - 3, cy - 2, 6, barH + 8, 2, 2);

    // Progress counter below
    p.setFont(cachedFont("Courier New", 13));
    p.setPen(QColor(170, 175, 190));
    p.drawText(QRect(cx - barW / 2, cy + 22, barW, 20), Qt::AlignCenter,
               QString("Ada is listening: %1 / %2").arg(mercy.successCount()).arg(mercy.requiredSuccesses()));
}

} // namespace hm
