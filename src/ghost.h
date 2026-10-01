#pragma once

#include <QPoint>

#include "entity.h"

struct Ghost : public Entity {
    enum class St { House, Exiting, Hunting, Frightened, Eyes };
    St st = St::House;
    int id = 0;        // 0 Blinky, 1 Pinky, 2 Inky, 3 Clyde
    float houseT = 0;  // seconds left in the house before release
    QPoint corner;     // scatter target
};
