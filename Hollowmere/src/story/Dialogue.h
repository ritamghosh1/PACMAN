#pragma once

#include <deque>
#include <functional>
#include <vector>
#include <QString>
#include <QColor>
#include <QPainter>
#include <QRect>
#include "audio/CineSfx.h"

namespace hm {

class AudioEngine;

struct DialogueLine {
    Speaker speaker = Speaker::Narrator;
    QString text;
};

// Typewriter text box shared by cutscenes and in-game story beats.
// Click / Space / Enter: first press completes the line, second press moves on.
// Speakers are told apart by colour, emblem, typeface and sound (tick timbre + a
// short motif when the speaker changes) instead of voice acting.
class DialogueBox {
public:
    void setAudio(AudioEngine* audio) { m_audio = audio; }

    void push(const DialogueLine& line);
    void push(Speaker who, const QString& text) { push(DialogueLine{who, text}); }
    void push(const std::vector<DialogueLine>& lines);
    void clear();

    bool active() const { return m_hasCurrent; }
    bool typing() const { return m_hasCurrent && m_shown < static_cast<float>(m_current.text.size()); }
    bool lineComplete() const { return m_hasCurrent && !typing(); }
    int pending() const { return static_cast<int>(m_queue.size()) + (m_hasCurrent ? 1 : 0); }

    // Returns true when the input was consumed by the dialogue.
    bool advance();
    void update(float dt);
    void render(QPainter& p, const QRect& bounds, float time) const;

    static QString displayName(Speaker s);
    static QColor accent(Speaker s);

    // Characters per second for the typewriter.
    void setSpeed(float cps) { m_cps = cps; }

    // Called when a line starts typing; index counts lines since clear().
    void setLineStartCallback(std::function<void(int)> cb) { m_onLineStart = std::move(cb); }

private:
    void startNext();
    std::vector<QString> wrap(const QString& text, const QFontMetrics& fm, int width) const;

    AudioEngine* m_audio = nullptr;
    std::deque<DialogueLine> m_queue;
    DialogueLine m_current;
    bool m_hasCurrent = false;
    bool m_hasLast = false;
    Speaker m_lastSpeaker = Speaker::Narrator;
    float m_shown = 0.0f;
    float m_pause = 0.0f;
    int m_lastTickIdx = -1;
    float m_sinceLine = 0.0f;
    float m_cps = 38.0f;
    int m_lineCounter = 0;
    std::function<void(int)> m_onLineStart;
};

} // namespace hm
