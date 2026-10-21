#pragma once

#include <QPainter>
#include <QRect>
#include "world/World.h"

namespace hm {

class Hud {
public:
    Hud() = default;

    void draw(QPainter& painter, const QRect& bounds, const World& world, float fps);

private:
    void drawCrosshair(QPainter& p, int cx, int cy, bool hasPrompt);
    void drawLanternGauge(QPainter& p, int x, int y, float oilPercent, bool isOn, bool isFocused);
    void drawStaminaBar(QPainter& p, int x, int y, float staminaRatio);
    void drawPrompts(QPainter& p, int cx, int cy, const QString& prompt, const QString& toast, float toastTimer);
    void drawObjectives(QPainter& p, int x, int y, int sigils, int keyFlags);
};

} // namespace hm
