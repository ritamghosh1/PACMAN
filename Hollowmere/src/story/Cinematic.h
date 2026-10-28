#pragma once

#include <functional>
#include <vector>
#include <QPainter>
#include <QPointF>
#include <QRect>
#include "Dialogue.h"

namespace hm {

class AudioEngine;
class Cinematic;

// Everything a shot's draw function needs. Art is drawn in a fixed 1600x900 space;
// Cinematic scales it to the window and applies the camera move.
struct ShotContext {
    float t = 0.0f;          // seconds since the shot began
    float fade = 1.0f;       // 0 black .. 1 fully visible (for shots that want to react)
    float shake = 0.0f;      // 0..1 impulse
    float seed = 0.0f;
    int beat = -1;           // index of the dialogue line currently on screen (-1 before the first)
    float beatT = 0.0f;      // seconds since that line started
};

constexpr float kStageW = 1600.0f;
constexpr float kStageH = 900.0f;

struct ShotCue {
    float time = 0.0f;
    std::function<void(Cinematic&)> fn;
};

struct Shot {
    QString id;
    std::function<void(QPainter&, const ShotContext&)> draw;
    std::vector<DialogueLine> lines;
    std::vector<ShotCue> cues;
    std::function<void(Cinematic&)> onEnter;
    std::function<void(Cinematic&, int)> onLine;     // fired as each dialogue line starts typing

    float lead = 1.4f;          // quiet seconds before the first line appears
    float autoDuration = 5.0f;  // used when the shot has no lines
    float outro = 0.9f;         // seconds held after the last line is dismissed
    float fadeIn = 1.0f;
    float fadeOut = 0.8f;

    // Slow camera push (Ken Burns): eased from (zoom0, pan0) to (zoom1, pan1) over camTime.
    float zoom0 = 1.0f, zoom1 = 1.10f;
    QPointF pan0{0, 0}, pan1{0, 0};
    float camTime = 16.0f;
};

// A scripted sequence of animated shots with typewriter dialogue.
class Cinematic {
public:
    explicit Cinematic(AudioEngine* audio);

    void addShot(Shot shot);
    void start();
    void update(float dt);
    void render(QPainter& p, const QRect& bounds);

    // Input. Both return true when consumed.
    bool onClick(const QPoint& pos, const QRect& bounds);
    bool onAdvanceKey();
    void onMouseMove(const QPoint& pos, const QRect& bounds);
    void skip();

    bool finished() const { return m_finished; }
    void setSkippable(bool s) { m_skippable = s; }

    // Cue helpers
    AudioEngine* audio() const { return m_audio; }
    void shake(float amount) { m_shake = std::max(m_shake, amount); }
    void flash(float amount) { m_flash = std::max(m_flash, amount); }
    float shotTime() const { return m_t; }
    int shotIndex() const { return m_index; }
    int shotCount() const { return static_cast<int>(m_shots.size()); }

private:
    enum class Phase { In, Run, Out };
    void enterShot(int index);
    void drawChrome(QPainter& p, const QRect& bounds, float fade);
    QRect skipButton(const QRect& bounds) const;

    AudioEngine* m_audio = nullptr;
    DialogueBox m_dialogue;
    std::vector<Shot> m_shots;
    std::vector<bool> m_cueFired;

    int m_index = -1;
    Phase m_phase = Phase::In;
    float m_t = 0.0f;
    float m_phaseT = 0.0f;
    float m_runT = 0.0f;
    float m_outroT = 0.0f;
    float m_clock = 0.0f;
    bool m_linesPushed = false;
    bool m_finished = false;
    bool m_skippable = true;
    bool m_hoverSkip = false;
    float m_shake = 0.0f;
    float m_flash = 0.0f;
    int m_beat = -1;
    float m_beatT = 0.0f;
};

} // namespace hm
