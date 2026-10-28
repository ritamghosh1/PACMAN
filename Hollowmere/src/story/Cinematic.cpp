#include "Cinematic.h"
#include "audio/AudioEngine.h"
#include "ui/FontCache.h"
#include <QFont>
#include <QLinearGradient>
#include <QRadialGradient>
#include <algorithm>
#include <cmath>

namespace hm {

namespace {
float smooth(float x) { x = std::clamp(x, 0.0f, 1.0f); return x * x * (3.0f - 2.0f * x); }
float hashf(int n) {
    uint32_t x = static_cast<uint32_t>(n) * 747796405u + 2891336453u;
    x = ((x >> ((x >> 28) + 4)) ^ x) * 277803737u;
    x = (x >> 22) ^ x;
    return (x & 0xFFFFFF) / 16777216.0f;
}
} // namespace

Cinematic::Cinematic(AudioEngine* audio) : m_audio(audio) {
    m_dialogue.setAudio(audio);
    m_dialogue.setLineStartCallback([this](int idx) {
        m_beat = idx;
        m_beatT = 0.0f;
        if (m_index >= 0 && m_index < static_cast<int>(m_shots.size()) && m_shots[m_index].onLine) m_shots[m_index].onLine(*this, idx);
    });
}

void Cinematic::addShot(Shot shot) { m_shots.push_back(std::move(shot)); }

void Cinematic::start() {
    m_finished = false;
    m_dialogue.clear();
    if (m_audio) m_audio->setGameplayMuted(true);
    enterShot(0);
}

void Cinematic::enterShot(int index) {
    m_index = index;
    if (index >= static_cast<int>(m_shots.size())) {
        m_finished = true;
        if (m_audio) {
            m_audio->stopAllBeds(1.5f);
            m_audio->setGameplayMuted(false);
        }
        return;
    }
    const Shot& s = m_shots[index];
    m_phase = Phase::In;
    m_t = m_phaseT = m_runT = m_outroT = 0.0f;
    m_linesPushed = false;
    m_beat = -1;
    m_beatT = 0.0f;
    m_cueFired.assign(s.cues.size(), false);
    m_dialogue.clear();
    if (s.onEnter) s.onEnter(*this);
}

void Cinematic::skip() {
    if (m_finished) return;
    m_dialogue.clear();
    if (m_audio) {
        m_audio->stopAllBeds(0.8f);
        m_audio->setGameplayMuted(false);
    }
    m_finished = true;
}

void Cinematic::update(float dt) {
    if (m_finished || m_index < 0) return;
    dt = std::min(dt, 0.1f);
    m_clock += dt;
    m_t += dt;
    m_beatT += dt;
    m_phaseT += dt;
    m_shake = std::max(0.0f, m_shake - dt * 1.8f);
    m_flash = std::max(0.0f, m_flash - dt * 3.0f);
    m_dialogue.update(dt);

    const Shot& s = m_shots[m_index];
    for (size_t i = 0; i < s.cues.size(); ++i) {
        if (!m_cueFired[i] && m_t >= s.cues[i].time) {
            m_cueFired[i] = true;
            if (s.cues[i].fn) s.cues[i].fn(*this);
        }
    }

    switch (m_phase) {
    case Phase::In:
        if (m_phaseT >= s.fadeIn) { m_phase = Phase::Run; m_phaseT = 0.0f; }
        break;
    case Phase::Run:
        m_runT += dt;
        if (s.lines.empty()) {
            if (m_runT >= s.autoDuration) { m_phase = Phase::Out; m_phaseT = 0.0f; }
        } else {
            if (!m_linesPushed && m_runT >= s.lead) {
                m_dialogue.push(s.lines);
                m_linesPushed = true;
            }
            if (m_linesPushed && !m_dialogue.active()) {
                m_outroT += dt;
                if (m_outroT >= s.outro) { m_phase = Phase::Out; m_phaseT = 0.0f; }
            }
        }
        break;
    case Phase::Out:
        if (m_phaseT >= s.fadeOut) enterShot(m_index + 1);
        break;
    }
}

bool Cinematic::onAdvanceKey() {
    if (m_finished || m_index < 0) return false;
    if (m_dialogue.active()) return m_dialogue.advance();
    // No text on screen: let the player hurry a silent beat along.
    if (m_phase == Phase::Run && m_shots[m_index].lines.empty() && m_runT > 0.8f) {
        m_runT = m_shots[m_index].autoDuration;
        return true;
    }
    return false;
}

QRect Cinematic::skipButton(const QRect& b) const {
    return QRect(b.right() - 150, b.top() + 18, 130, 34);
}

bool Cinematic::onClick(const QPoint& pos, const QRect& bounds) {
    if (m_finished) return false;
    if (m_skippable && skipButton(bounds).contains(pos)) {
        if (m_audio) m_audio->playUi(1);
        skip();
        return true;
    }
    return onAdvanceKey();
}

void Cinematic::onMouseMove(const QPoint& pos, const QRect& bounds) {
    m_hoverSkip = m_skippable && skipButton(bounds).contains(pos);
}

void Cinematic::render(QPainter& p, const QRect& bounds) {
    if (m_finished || m_index < 0 || m_index >= static_cast<int>(m_shots.size())) {
        p.fillRect(bounds, Qt::black);
        return;
    }
    const Shot& s = m_shots[m_index];
    float fade = 1.0f;
    if (m_phase == Phase::In) fade = smooth(m_phaseT / std::max(0.01f, s.fadeIn));
    else if (m_phase == Phase::Out) fade = 1.0f - smooth(m_phaseT / std::max(0.01f, s.fadeOut));

    p.save();
    p.fillRect(bounds, Qt::black);
    p.setClipRect(bounds);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Fit the 1600x900 stage so it always covers the window.
    const float scale = std::max(bounds.width() / kStageW, bounds.height() / kStageH);
    float camT = smooth(m_t / std::max(0.1f, s.camTime));
    float zoom = s.zoom0 + (s.zoom1 - s.zoom0) * camT;
    QPointF pan = s.pan0 + (s.pan1 - s.pan0) * camT;
    float shakeX = m_shake > 0.0f ? std::sin(m_clock * 61.0f) * m_shake * 14.0f : 0.0f;
    float shakeY = m_shake > 0.0f ? std::cos(m_clock * 53.0f) * m_shake * 11.0f : 0.0f;
    // Slow handheld sway keeps still shots from feeling like slideshows.
    float swayX = std::sin(m_clock * 0.55f) * 3.0f, swayY = std::cos(m_clock * 0.43f) * 2.5f;

    p.translate(bounds.center());
    p.scale(scale * zoom, scale * zoom);
    p.translate(-kStageW * 0.5f + pan.x() + shakeX + swayX, -kStageH * 0.5f + pan.y() + shakeY + swayY);

    ShotContext ctx;
    ctx.t = m_t;
    ctx.fade = fade;
    ctx.shake = m_shake;
    ctx.beat = m_beat;
    ctx.beatT = m_beatT;
    if (s.draw) s.draw(p, ctx);
    p.restore();

    if (m_flash > 0.0f) p.fillRect(bounds, QColor(235, 240, 255, static_cast<int>(std::min(1.0f, m_flash) * 235)));

    drawChrome(p, bounds, fade);
    m_dialogue.render(p, bounds, m_clock);
}

void Cinematic::drawChrome(QPainter& p, const QRect& b, float fade) {
    p.save();
    // Film vignette
    QRadialGradient vg(b.center(), std::max(b.width(), b.height()) * 0.72f);
    vg.setColorAt(0.0, QColor(0, 0, 0, 0));
    vg.setColorAt(0.62, QColor(0, 0, 0, 55));
    vg.setColorAt(1.0, QColor(0, 0, 0, 215));
    p.fillRect(b, vg);

    // Film grain: a few hundred specks that reshuffle every frame.
    int frame = static_cast<int>(m_clock * 24.0f);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 160; ++i) {
        float x = hashf(frame * 977 + i * 31) * b.width();
        float y = hashf(frame * 613 + i * 57 + 5) * b.height();
        int a = 14 + static_cast<int>(hashf(frame + i * 7) * 26);
        p.setBrush(QColor(255, 255, 255, a));
        p.drawRect(QRectF(x, y, 1.6, 1.6));
    }

    // Letterbox bars slide in during the first shot's fade.
    float barsIn = smooth(m_clock / 2.2f);
    int barH = static_cast<int>(b.height() * 0.085f * barsIn);
    p.fillRect(QRect(b.left(), b.top(), b.width(), barH), Qt::black);
    p.fillRect(QRect(b.left(), b.bottom() - barH + 1, b.width(), barH), Qt::black);

    // Dip to black between shots (drawn after bars so the bars don't pop).
    if (fade < 1.0f) p.fillRect(b, QColor(0, 0, 0, static_cast<int>((1.0f - fade) * 255)));

    if (m_skippable) {
        QRect sb = skipButton(b);
        p.setFont(cachedFont("Courier New", 15, true));
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(QColor(200, 190, 170, m_hoverSkip ? 230 : 110), 1.2));
        p.setBrush(QColor(0, 0, 0, m_hoverSkip ? 170 : 90));
        p.drawRoundedRect(sb, 5, 5);
        p.setPen(QColor(225, 215, 195, m_hoverSkip ? 255 : 150));
        p.drawText(sb, Qt::AlignCenter, QString::fromUtf8("SKIP  \xE2\x96\xB8\xE2\x96\xB8"));
    }
    p.restore();
}

} // namespace hm
