#include "InventoryWidget.h"
#include <QFont>

namespace hm {

InventoryOverlay::InventoryOverlay() = default;

void InventoryOverlay::init(const QRect&) {
    m_selectedSlot = 0;
}

void InventoryOverlay::selectNext() {
    m_selectedSlot = (m_selectedSlot + 1) % 6;
}

void InventoryOverlay::selectPrev() {
    m_selectedSlot = (m_selectedSlot + 5) % 6;
}

void InventoryOverlay::render(QPainter& p, const QRect& bounds, const Inventory& inv) {
    // Bottom-center quickbar
    int slotSize = 64;
    int gap = 12;
    int totalW = 6 * slotSize + 5 * gap;
    int startX = (bounds.width() - totalW) / 2;
    int startY = bounds.height() - slotSize - 25;

    // Dark bar backing
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(8, 10, 14, 180));
    p.drawRoundedRect(startX - 15, startY - 12, totalW + 30, slotSize + 24, 8, 8);

    QFont slotNumFont("Courier New", 9, QFont::Bold);
    QFont itemNameFont("Arial", 8, QFont::Normal);

    for (int i = 0; i < 6; ++i) {
        int sx = startX + i * (slotSize + gap);
        QRect slotRect(sx, startY, slotSize, slotSize);

        if (i == m_selectedSlot) {
            p.setPen(QPen(QColor(220, 180, 80), 2.0f));
            p.setBrush(QColor(35, 38, 50, 220));
        } else {
            p.setPen(QPen(QColor(60, 65, 80), 1.0f));
            p.setBrush(QColor(16, 18, 24, 200));
        }
        p.drawRoundedRect(slotRect, 6, 6);

        // Slot number (1-6)
        p.setFont(slotNumFont);
        p.setPen(QColor(140, 145, 160));
        p.drawText(sx + 6, startY + 16, QString::number(i + 1));

        // Slot contents
        if (i < static_cast<int>(inv.itemSlots().size())) {
            const auto& item = inv.itemSlots()[i];
            p.setFont(itemNameFont);
            p.setPen(QColor(240, 240, 245));
            p.drawText(QRect(sx + 4, startY + 20, slotSize - 8, slotSize - 24), Qt::AlignCenter | Qt::TextWordWrap, QString::fromStdString(item.name));

            if (item.count > 1) {
                p.setFont(slotNumFont);
                p.setPen(QColor(220, 180, 80));
                p.drawText(QRect(sx, startY + slotSize - 18, slotSize - 6, 14), Qt::AlignRight, QString("x%1").arg(item.count));
            }
        }
    }

    // Keyring badge display on right
    int krX = startX + totalW + 25;
    QRect krRect(krX, startY, 90, slotSize);
    p.setPen(QPen(QColor(100, 80, 40), 1.0f));
    p.setBrush(QColor(20, 16, 12, 200));
    p.drawRoundedRect(krRect, 6, 6);

    p.setFont(itemNameFont);
    p.setPen(QColor(220, 190, 120));
    p.drawText(QRect(krX, startY + 8, 90, 20), Qt::AlignCenter, "KEYRING");
    p.setPen(QColor(180, 180, 190));
    p.drawText(QRect(krX, startY + 30, 90, 25), Qt::AlignCenter, QString("%1 Keys").arg(inv.keys().size()));
}

} // namespace hm
