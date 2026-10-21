#pragma once

#include <QRect>
#include <QPainter>
#include <QString>
#include "gameplay/Mercy.h"

namespace hm {

class RhythmOverlay {
public:
    RhythmOverlay();

    void render(QPainter& p, const QRect& bounds, const MercySystem& mercy);
};

} // namespace hm
