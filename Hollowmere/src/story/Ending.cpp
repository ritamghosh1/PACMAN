#include "Ending.h"
#include <QFont>
#include <algorithm>

namespace hm {

EndingCutscene::EndingCutscene(EndingType ending, AudioEngine* audio)
    : m_ending(ending), m_audio(audio) {
    init();
}

void EndingCutscene::init() {
    setDuration(60.0f); // 60s total for ending scene + credits scroll
    setupEndingScenes();
}

void EndingCutscene::setupEndingScenes() {
    QString titleStr;
    QString subStr;
    QColor themeColor;

    switch (m_ending) {
    case EndingType::E1_Release:
        titleStr = "ENDING I: THE CRIMSON DAWN";
        subStr = "Nora laid her lantern down in the morning light. The souls of the children walked into the dawn.";
        themeColor = QColor(220, 180, 120);
        break;
    case EndingType::E2_Consumed:
        titleStr = "ENDING II: SURRENDER TO THE HOLLOW";
        subStr = "The cold darkness claimed Nora. Her memories dissolved into the foundation of Hollowmere.";
        themeColor = QColor(160, 80, 80);
        break;
    case EndingType::E3_Loop:
        titleStr = "ENDING III: THE ENDLESS CORRIDOR";
        subStr = "She ran through the front gate, only to awaken back in the Foyer. The cycle repeats forever.";
        themeColor = QColor(120, 130, 160);
        break;
    case EndingType::E4_Martyr:
        titleStr = "ENDING IV: THE SACRED ASHES";
        subStr = "The sanitarium burned to cinder. Nora remained behind so that no shadow would ever escape.";
        themeColor = QColor(220, 120, 40);
        break;
    case EndingType::E5_Revelation:
        titleStr = "SECRET ENDING: THE WARDEN'S LEDGER";
        subStr = "Every name. Every child. Every experiment. Nora carried Dr. Crane's ledger to the authorities.";
        themeColor = QColor(240, 220, 130);
        break;
    }

    // 1. Narrative Art & Title layer (0s - 25s)
    addLayer({
        "EndingCinematic",
        0,
        [titleStr, subStr, themeColor](QPainter& p, const QRect& bounds, float t, float, const QPointF&, float) {
            if (t > 25.0f) return;
            float alpha = 1.0f;
            if (t < 2.0f) alpha = t / 2.0f;
            else if (t > 22.0f) alpha = (25.0f - t) / 3.0f;

            p.fillRect(bounds, Qt::black);

            int cx = bounds.width() / 2;
            int cy = bounds.height() / 2;

            QFont titleF("Georgia", 26, QFont::Bold);
            p.setFont(titleF);
            p.setPen(QColor(themeColor.red(), themeColor.green(), themeColor.blue(), static_cast<int>(255 * alpha)));
            p.drawText(QRect(cx - 400, cy - 80, 800, 60), Qt::AlignCenter, titleStr);

            QFont subF("Georgia", 15, QFont::Normal);
            p.setFont(subF);
            p.setPen(QColor(210, 210, 215, static_cast<int>(230 * alpha)));
            p.drawText(QRect(cx - 350, cy + 10, 700, 100), Qt::AlignCenter | Qt::TextWordWrap, subStr);
        }
    });

    // 2. Credits Scroll Layer (24s - 60s)
    addLayer({
        "CreditsScroll",
        1,
        [](QPainter& p, const QRect& bounds, float t, float, const QPointF&, float) {
            if (t < 24.0f) return;
            float alpha = std::min(1.0f, (t - 24.0f) / 2.0f);

            p.fillRect(bounds, Qt::black);

            float scrollY = bounds.height() - (t - 25.0f) * 45.0f;
            int cx = bounds.width() / 2;

            std::vector<std::pair<QString, QString>> credits = {
                {"HOLLOWMERE", "A Psychological Horror Experience"},
                {"DIRECTOR & ARCHITECTURE", "Computer Graphics Systems Lab"},
                {"3D GRAPHICS & SHADER PIPELINE", "OpenGL 3.3 Core Profile Engine"},
                {"PROCEDURAL AUDIO SYNTHESIS", "FDN Reverb, Woodworth ITD & Additive Voice"},
                {"GHOST AI SENSORY SYSTEM", "Line-of-Sight Raycasting & Acoustic Bus"},
                {"STORY & LORE AUTHORING", "The Tragedy of Blackwood Orphanage"},
                {"SPECIAL THANKS", "Qt 5.15.2, Modern Web & Science Plugins"},
                {"THANK YOU FOR PLAYING", "All procedural assets generated in real time."}
            };

            float y = scrollY;
            for (const auto& item : credits) {
                if (y > -100 && y < bounds.height() + 100) {
                    QFont headerFont("Georgia", 16, QFont::Bold);
                    p.setFont(headerFont);
                    p.setPen(QColor(230, 200, 140, static_cast<int>(255 * alpha)));
                    p.drawText(QRect(cx - 300, static_cast<int>(y), 600, 30), Qt::AlignCenter, item.first);

                    QFont subFont("Monospace", 12, QFont::Normal);
                    p.setFont(subFont);
                    p.setPen(QColor(180, 180, 190, static_cast<int>(220 * alpha)));
                    p.drawText(QRect(cx - 300, static_cast<int>(y + 32), 600, 30), Qt::AlignCenter, item.second);
                }
                y += 90.0f;
            }
        }
    });

    // Audio cue for ending music
    if (m_audio) {
        addAudioCue(0.5f, [this]() {
            m_audio->playEndingScore(static_cast<int>(m_ending));
        });
    }
}

} // namespace hm
