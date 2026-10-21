#pragma once

#include <QString>
#include <QColor>
#include <vector>

namespace hm {

struct SubtitleEntry {
    QString speaker;
    QString text;
    QColor color;
    float startTime;
    float duration;
};

class SubtitleSystem {
public:
    static SubtitleSystem& instance();

    void show(const QString& speaker, const QString& text, float duration = 4.0f, const QColor& color = QColor(240, 240, 240));
    void update(float dt);
    void clear();

    bool hasActive() const { return m_active; }
    QString currentSpeaker() const { return m_speaker; }
    QString currentText() const { return m_text; }
    QColor currentColor() const { return m_color; }
    float alpha() const;

private:
    SubtitleSystem() = default;

    bool m_active = false;
    QString m_speaker;
    QString m_text;
    QColor m_color = QColor(240, 240, 240);
    float m_timer = 0.0f;
    float m_duration = 0.0f;
};

} // namespace hm
