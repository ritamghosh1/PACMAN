#pragma once

// Small helpers shared by the intro and ending scenes.

#include <QColor>
#include <QPainter>
#include <QString>
#include <vector>
#include "Cinematic.h"
#include "audio/AudioEngine.h"

namespace hm {
namespace scenekit {

enum BedTag { BedRain = 101, BedFire = 102, BedCar = 103, BedWind = 104, BedPad = 105, BedBox = 106, BedExtra = 107 };

constexpr float W = kStageW;
constexpr float H = kStageH;

// Null-safe audio helpers so scenes also run in the silent preview tool.
inline void sfx(Cinematic& c, const std::vector<int16_t>& s, float vol = 1.0f, float pan = 0.0f) { if (c.audio()) c.audio()->playCine(s, vol, pan); }
inline void bed(Cinematic& c, BedTag tag, const std::vector<int16_t>& s, float vol, float fade = 2.0f) { if (c.audio()) c.audio()->startBed(tag, s, vol, fade); }
inline void bedStop(Cinematic& c, BedTag tag, float fade = 1.5f) { if (c.audio()) c.audio()->stopBed(tag, fade); }
inline void bedVol(Cinematic& c, BedTag tag, float vol, float ramp = 1.0f) { if (c.audio()) c.audio()->setBedVolume(tag, vol, ramp); }

inline QString U(const char* s) { return QString::fromUtf8(s); }
inline void fillAll(QPainter& p, const QColor& c) { p.fillRect(QRectF(-200, -200, W + 400, H + 400), c); }

// The windscreen drive from the intro, reused by the "endless drive" ending.
void drawDrive(QPainter& p, const ShotContext& c);

} // namespace scenekit
} // namespace hm
