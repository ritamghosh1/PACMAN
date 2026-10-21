#pragma once

#include <vector>
#include <functional>
#include <QString>
#include <QPainter>
#include <QRect>
#include <QColor>
#include <QPointF>

namespace hm {

enum class Easing {
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseOutCubic
};

struct CutsceneLayer {
    QString id;
    int depth = 0; // for parallax sorting
    std::function<void(QPainter& p, const QRect& bounds, float t, float alpha, const QPointF& offset, float zoom)> drawFunc;
};

struct CutsceneAudioCue {
    float timestamp = 0.0f;
    std::function<void()> trigger;
    bool triggered = false;
};

struct CutsceneSubtitleCue {
    float timestamp = 0.0f;
    float duration = 3.0f;
    QString speaker;
    QString text;
    QColor color = QColor(240, 240, 240);
    bool triggered = false;
};

class Cutscene {
public:
    Cutscene();
    virtual ~Cutscene() = default;

    void setDuration(float seconds) { m_totalDuration = seconds; }
    float duration() const { return m_totalDuration; }

    void addLayer(const CutsceneLayer& layer);
    void addAudioCue(float time, std::function<void()> trigger);
    void addSubtitle(float time, float duration, const QString& speaker, const QString& text, const QColor& color = QColor(240, 240, 240));

    void reset();
    void update(float dt, bool spaceHeld);
    void render(QPainter& p, const QRect& bounds);

    bool isFinished() const { return m_currentTime >= m_totalDuration; }
    float currentTime() const { return m_currentTime; }
    float skipProgress() const { return m_skipHoldTimer / m_skipHoldRequired; }

protected:
    float m_totalDuration = 30.0f;
    float m_currentTime = 0.0f;
    float m_skipHoldTimer = 0.0f;
    const float m_skipHoldRequired = 1.0f; // 1s hold space to skip

    float m_camShake = 0.0f;
    float m_camZoom = 1.0f;
    QPointF m_camOffset{0.0f, 0.0f};

    std::vector<CutsceneLayer> m_layers;
    std::vector<CutsceneAudioCue> m_audioCues;
    std::vector<CutsceneSubtitleCue> m_subtitles;

    void drawVignetteAndGrain(QPainter& p, const QRect& bounds);
    void drawRainAndLightning(QPainter& p, const QRect& bounds, float t);
};

} // namespace hm
