#pragma once

#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QColor>

namespace hm {
namespace cineart {

// Stage is 1600x900. Every function is a pure function of time so shots can be
// scrubbed, skipped and previewed deterministically.

float hash(int n, int seed = 0);                 // [0,1)
float vnoise(float x, int seed = 0);             // smooth 1D value noise [0,1]
float smooth(float x);
float mixf(float a, float b, float t);
QColor mix(const QColor& a, const QColor& b, float t);
QColor alpha(const QColor& c, float a01);
float lightning(float t, const float* strikes, int n);   // 0..1 flash intensity with double flicker

void glow(QPainter& p, QPointF c, float radius, const QColor& col, float a01);
void sky(QPainter& p, const QColor& top, const QColor& bottom, float horizonY = 900.0f);
void clouds(QPainter& p, float t, const QColor& col, float a01, float y, float h);
void fog(QPainter& p, float t, const QColor& col, float a01, float y0, float y1);
void rain(QPainter& p, float t, float density, float wind, float speed, float a01 = 1.0f);
void embers(QPainter& p, float t, QRectF area, int count, float speed, const QColor& col);
void flames(QPainter& p, float t, QRectF area, float intensity, int seed = 0);
void candle(QPainter& p, QPointF base, float t, float scale = 1.0f, int seed = 0);
void rainOnGlass(QPainter& p, float t, QRectF area, int drops, float a01 = 1.0f);

// Silhouettes
void bust(QPainter& p, QPointF neckBase, float scale, const QColor& col, float sway);          // head & shoulders from behind
void figureBack(QPainter& p, QPointF feet, float scale, const QColor& col, float sway);        // full body from behind (long coat)
void child(QPainter& p, QPointF feet, float scale, const QColor& col, const QColor& accent, float sway);
void hand(QPainter& p, QPointF wrist, float scale, float angleDeg, const QColor& col, float curl);

// Set pieces
void manor(QPainter& p, float t, QPointF base, float scale, float litProgress, const QColor& sil, float flash);
void gate(QPainter& p, QRectF area, float open01, const QColor& iron);
void pines(QPainter& p, float t, float y, float h, const QColor& col, float scroll, int seed);
void road(QPainter& p, float t, float speed, float horizonY);
void corridor(QPainter& p, float t, QPointF vp, const QColor& wall, const QColor& floor);
void musicBox(QPainter& p, QPointF c, float t, float lid01, float scale);
void phone(QPainter& p, QPointF c, float angleDeg, float scale, bool lit, float t);

} // namespace cineart
} // namespace hm
