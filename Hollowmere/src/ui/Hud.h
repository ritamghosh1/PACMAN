#pragma once

#include <QPainter>
#include <QRect>
#include <QImage>
#include "world/World.h"

namespace hm {

class Hud {
public:
    Hud() = default;

    // viewProj lets the HUD pin a marker on the objective when it is on screen.
    void draw(QPainter& painter, const QRect& bounds, const World& world, float fps, const Mat4& viewProj, float time, bool showDebug);
    void update(float dt);
    void showLocation(const QString& name);
    void toggleMap() { m_mapVisible = !m_mapVisible; }
    void resetSession() { m_sessionTime = 0.0f; m_bannerTime = 99.0f; }

private:
    void drawCrosshair(QPainter& p, int cx, int cy, bool hasPrompt);
    void drawPrompts(QPainter& p, int cx, int cy, const QString& prompt, const QString& toast, float toastTimer);
    void drawLanternGauge(QPainter& p, int x, int y, float oilPercent, bool isOn, bool isFocused);
    void drawStatus(QPainter& p, int x, int y, const World& world);
    void drawObjective(QPainter& p, int x, int y, const World& world);
    void drawCompass(QPainter& p, int cx, int y, const World& world);
    void drawMarker(QPainter& p, const QRect& b, const World& world, const Mat4& viewProj, float time);
    void drawMinimap(QPainter& p, const QRect& b, const World& world, float time);
    void drawBanner(QPainter& p, const QRect& b);
    void drawControls(QPainter& p, int x, int y);

    QImage m_mapImage;
    int m_mapVersion = -1;
    int m_mapDoorSig = -1;
    bool m_mapVisible = true;
    float m_sessionTime = 0.0f;
    QString m_bannerText;
    float m_bannerTime = 99.0f;
};

} // namespace hm
