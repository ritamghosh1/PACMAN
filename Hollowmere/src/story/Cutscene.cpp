#include "Cutscene.h"
#include "Subtitles.h"
#include <algorithm>
#include <cmath>
#include <QLinearGradient>
#include <QPen>
#include <QFont>

namespace hm {

Cutscene::Cutscene() = default;

void Cutscene::addLayer(const CutsceneLayer& layer) {
    m_layers.push_back(layer);
    // Sort layers by depth (back to front)
    std::sort(m_layers.begin(), m_layers.end(), [](const CutsceneLayer& a, const CutsceneLayer& b) {
        return a.depth < b.depth;
    });
}

void Cutscene::addAudioCue(float time, std::function<void()> trigger) {
    m_audioCues.push_back({time, trigger, false});
}

void Cutscene::addSubtitle(float time, float duration, const QString& speaker, const QString& text, const QColor& color) {
    m_subtitles.push_back({time, duration, speaker, text, color, false});
}

void Cutscene::reset() {
    m_currentTime = 0.0f;
    m_skipHoldTimer = 0.0f;
    m_camShake = 0.0f;
    m_camZoom = 1.0f;
    m_camOffset = QPointF(0.0f, 0.0f);
    for (auto& cue : m_audioCues) cue.triggered = false;
    for (auto& sub : m_subtitles) sub.triggered = false;
}

void Cutscene::update(float dt, bool spaceHeld) {
    if (spaceHeld) {
        m_skipHoldTimer += dt;
        if (m_skipHoldTimer >= m_skipHoldRequired) {
            m_currentTime = m_totalDuration;
            return;
        }
    } else {
        m_skipHoldTimer = std::max(0.0f, m_skipHoldTimer - dt * 2.0f);
    }

    m_currentTime += dt;

    // Trigger audio cues
    for (auto& cue : m_audioCues) {
        if (!cue.triggered && m_currentTime >= cue.timestamp) {
            cue.triggered = true;
            if (cue.trigger) cue.trigger();
        }
    }

    // Trigger subtitles
    for (auto& sub : m_subtitles) {
        if (!sub.triggered && m_currentTime >= sub.timestamp) {
            sub.triggered = true;
            SubtitleSystem::instance().show(sub.speaker, sub.text, sub.duration, sub.color);
        }
    }

    // Decay camera shake
    if (m_camShake > 0.0f) {
        m_camShake = std::max(0.0f, m_camShake - dt * 5.0f);
    }
}

void Cutscene::render(QPainter& p, const QRect& bounds) {
    p.save();
    p.fillRect(bounds, Qt::black);

    // Apply camera shake & offset
    float shakeX = (m_camShake > 0.0f) ? std::sin(m_currentTime * 45.0f) * m_camShake * 12.0f : 0.0f;
    float shakeY = (m_camShake > 0.0f) ? std::cos(m_currentTime * 38.0f) * m_camShake * 10.0f : 0.0f;
    QPointF effectiveOffset = m_camOffset + QPointF(shakeX, shakeY);

    // Render layers
    for (const auto& layer : m_layers) {
        if (layer.drawFunc) {
            p.save();
            layer.drawFunc(p, bounds, m_currentTime, 1.0f, effectiveOffset, m_camZoom);
            p.restore();
        }
    }

    // Rain and atmospheric lightning
    drawRainAndLightning(p, bounds, m_currentTime);

    // Vignette and vintage film grain
    drawVignetteAndGrain(p, bounds);

    // Skip prompt if user is holding or touching space
    if (m_skipHoldTimer > 0.05f) {
        int barW = 200;
        int barH = 6;
        int bx = bounds.width() - barW - 30;
        int by = bounds.height() - 30;

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(40, 40, 40, 180));
        p.drawRoundedRect(bx - 5, by - 22, barW + 10, barH + 28, 4, 4);

        p.setFont(QFont("Monospace", 9, QFont::Bold));
        p.setPen(QColor(200, 200, 200));
        p.drawText(bx, by - 6, "HOLD SPACE TO SKIP");

        p.setBrush(QColor(70, 70, 70));
        p.drawRect(bx, by, barW, barH);

        p.setBrush(QColor(220, 50, 50));
        p.drawRect(bx, by, static_cast<int>(barW * skipProgress()), barH);
    }

    p.restore();
}

void Cutscene::drawVignetteAndGrain(QPainter& p, const QRect& bounds) {
    // Subtle circular vignette
    QRadialGradient grad(bounds.center(), std::max(bounds.width(), bounds.height()) * 0.7f);
    grad.setColorAt(0.0f, QColor(0, 0, 0, 0));
    grad.setColorAt(0.7f, QColor(0, 0, 0, 70));
    grad.setColorAt(1.0f, QColor(0, 0, 0, 220));
    p.fillRect(bounds, grad);
}

void Cutscene::drawRainAndLightning(QPainter& p, const QRect& bounds, float t) {
    // Slanted rain lines
    p.setPen(QPen(QColor(160, 180, 210, 40), 1.0f));
    int numDrops = 80;
    for (int i = 0; i < numDrops; ++i) {
        float xSeed = std::fmod((float)(i * 37) + t * 450.0f, (float)bounds.width());
        float ySeed = std::fmod((float)(i * 73) + t * 900.0f, (float)bounds.height());
        p.drawLine(QPointF(xSeed, ySeed), QPointF(xSeed - 6.0f, ySeed + 24.0f));
    }
}

} // namespace hm
