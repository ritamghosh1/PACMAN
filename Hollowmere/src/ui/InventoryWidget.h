#pragma once

#include <QRect>
#include <QPainter>
#include <QString>
#include "gameplay/Inventory.h"
#include "gameplay/Crafting.h"

namespace hm {

class InventoryOverlay {
public:
    InventoryOverlay();

    void init(const QRect& bounds);
    void render(QPainter& p, const QRect& bounds, const Inventory& inv);

    void selectNext();
    void selectPrev();
    int selectedSlot() const { return m_selectedSlot; }
    void selectSlot(int slot) { m_selectedSlot = slot; }

private:
    int m_selectedSlot = 0;
};

} // namespace hm
