#include "Subtitles.h"
#include <algorithm>

namespace hm {

SubtitleSystem& SubtitleSystem::instance() {
    static SubtitleSystem s_instance;
    return s_instance;
}

void SubtitleSystem::show(const QString& speaker, const QString& text, float duration, const QColor& color) {
    m_active = true;
    m_speaker = speaker;
    m_text = text;
    m_duration = duration;
    m_timer = 0.0f;
    m_color = color;
}

void SubtitleSystem::update(float dt) {
    if (!m_active) return;
    m_timer += dt;
    if (m_timer >= m_duration) {
        m_active = false;
    }
}

void SubtitleSystem::clear() {
    m_active = false;
    m_speaker.clear();
    m_text.clear();
    m_timer = 0.0f;
}

float SubtitleSystem::alpha() const {
    if (!m_active || m_duration <= 0.0f) return 0.0f;
    // Fade in over 0.25s, fade out over 0.4s
    if (m_timer < 0.25f) {
        return m_timer / 0.25f;
    }
    float rem = m_duration - m_timer;
    if (rem < 0.4f) {
        return std::max(0.0f, rem / 0.4f);
    }
    return 1.0f;
}

} // namespace hm
