#include "IntroMovie.h"
#include <QPainterPath>
#include <QPolygonF>
#include <cmath>
#include <algorithm>

namespace hm {

IntroMovie::IntroMovie(AudioEngine* audio)
    : m_audio(audio) {
    init();
}

void IntroMovie::init() {
    setDuration(80.0f); // 80 seconds intro
    setupScenes();
}

void IntroMovie::setupScenes() {
    // 1. Scene 1: Fire Memory (0s - 18s)
    addLayer({
        "FireMemory",
        0,
        [](QPainter& p, const QRect& bounds, float t, float, const QPointF& offset, float) {
            if (t > 18.0f) return;
            float alpha = (t < 15.0f) ? 1.0f : (18.0f - t) / 3.0f;

            // Orange/red flame ambient glow
            float flicker = 0.85f + 0.15f * std::sin(t * 12.0f);
            QLinearGradient flameGrad(0, bounds.height(), 0, 0);
            flameGrad.setColorAt(0.0f, QColor(220, 60, 10, static_cast<int>(200 * alpha * flicker)));
            flameGrad.setColorAt(0.6f, QColor(140, 20, 5, static_cast<int>(150 * alpha * flicker)));
            flameGrad.setColorAt(1.0f, QColor(10, 0, 0, static_cast<int>(255 * alpha)));
            p.fillRect(bounds, flameGrad);

            // Silhouette of burning rafters and nursery crib
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(15, 5, 5, static_cast<int>(255 * alpha)));

            // Nursery Crib silhouette
            float cx = bounds.width() * 0.5f + offset.x();
            float cy = bounds.height() * 0.65f + offset.y();
            p.drawRect(static_cast<int>(cx - 60), static_cast<int>(cy - 30), 120, 60);
            for (int r = 0; r < 8; ++r) {
                p.drawRect(static_cast<int>(cx - 50 + r * 14), static_cast<int>(cy - 60), 6, 35);
            }

            // Smoke embers rising
            p.setBrush(QColor(255, 180, 50, static_cast<int>(180 * alpha)));
            for (int i = 0; i < 30; ++i) {
                float ex = std::fmod((float)(i * 47) + std::sin(t * 2.0f + i) * 30.0f, (float)bounds.width());
                float ey = bounds.height() - std::fmod(t * 120.0f + (float)(i * 31), (float)bounds.height());
                p.drawEllipse(QPointF(ex, ey), 3, 3);
            }
        }
    });

    // 2. Scene 2: Nora with the letter (18s - 38s)
    addLayer({
        "LetterScene",
        1,
        [](QPainter& p, const QRect& bounds, float t, float, const QPointF& offset, float) {
            if (t < 17.5f || t > 38.0f) return;
            float alpha = 1.0f;
            if (t < 19.5f) alpha = (t - 17.5f) / 2.0f;
            else if (t > 36.0f) alpha = (38.0f - t) / 2.0f;

            // Cold desk lamp pool
            QRadialGradient lamp(bounds.width() * 0.5f + offset.x(), bounds.height() * 0.55f + offset.y(), bounds.width() * 0.35f);
            lamp.setColorAt(0.0f, QColor(220, 210, 180, static_cast<int>(140 * alpha)));
            lamp.setColorAt(0.8f, QColor(20, 22, 25, static_cast<int>(220 * alpha)));
            lamp.setColorAt(1.0f, QColor(5, 5, 8, static_cast<int>(255 * alpha)));
            p.fillRect(bounds, lamp);

            // Paper letter on desk
            float lx = bounds.width() * 0.5f - 110 + offset.x();
            float ly = bounds.height() * 0.45f + offset.y();
            p.setPen(QColor(60, 50, 40, static_cast<int>(200 * alpha)));
            p.setBrush(QColor(240, 235, 215, static_cast<int>(230 * alpha)));
            p.drawRect(static_cast<int>(lx), static_cast<int>(ly), 220, 160);

            // Red wax seal
            p.setBrush(QColor(180, 20, 20, static_cast<int>(240 * alpha)));
            p.drawEllipse(QPointF(lx + 190, ly + 130), 16, 16);

            // Shaking hand holding the letter
            p.setBrush(QColor(18, 18, 22, static_cast<int>(255 * alpha)));
            p.drawEllipse(QPointF(lx + 10, ly + 90), 30, 24);
        }
    });

    // 3. Scene 3: Car driving through storm (38s - 58s)
    addLayer({
        "CarStorm",
        2,
        [](QPainter& p, const QRect& bounds, float t, float, const QPointF& offset, float) {
            if (t < 37.5f || t > 58.0f) return;
            float alpha = 1.0f;
            if (t < 39.5f) alpha = (t - 37.5f) / 2.0f;
            else if (t > 56.0f) alpha = (58.0f - t) / 2.0f;

            // Pitch black rainy highway
            p.fillRect(bounds, QColor(4, 6, 8, static_cast<int>(255 * alpha)));

            // Headlight beams cutting through fog
            QPolygonF beamLeft;
            float carX = bounds.width() * 0.5f + offset.x();
            float carY = bounds.height() * 0.75f + offset.y();
            beamLeft << QPointF(carX - 80, carY) << QPointF(carX - 350, carY - 300) << QPointF(carX - 100, carY - 300);

            QPolygonF beamRight;
            beamRight << QPointF(carX + 80, carY) << QPointF(carX + 100, carY - 300) << QPointF(carX + 350, carY - 300);

            QLinearGradient beamGrad(carX, carY, carX, carY - 300);
            beamGrad.setColorAt(0.0f, QColor(255, 250, 200, static_cast<int>(180 * alpha)));
            beamGrad.setColorAt(1.0f, QColor(200, 220, 255, static_cast<int>(10 * alpha)));

            p.setPen(Qt::NoPen);
            p.setBrush(beamGrad);
            p.drawPolygon(beamLeft);
            p.drawPolygon(beamRight);

            // Dashboard silhouette
            p.setBrush(QColor(10, 10, 14, static_cast<int>(255 * alpha)));
            p.drawRect(0, static_cast<int>(carY + 20), bounds.width(), bounds.height());

            // Windshield wiper sweep
            float sweepAngle = std::sin(t * 3.5f) * 45.0f;
            p.save();
            p.translate(carX, carY + 20);
            p.rotate(sweepAngle);
            p.setPen(QPen(QColor(30, 30, 35, static_cast<int>(255 * alpha)), 5.0f));
            p.drawLine(0, 0, 0, -180);
            p.restore();
        }
    });

    // 4. Scene 4: Manor Reveal with Lightning & Gate Slam (58s - 75s)
    addLayer({
        "ManorSilhouette",
        3,
        [this](QPainter& p, const QRect& bounds, float t, float, const QPointF& offset, float) {
            if (t < 57.5f || t > 75.0f) return;
            float alpha = 1.0f;
            if (t < 59.5f) alpha = (t - 57.5f) / 2.0f;
            else if (t > 73.0f) alpha = (75.0f - t) / 2.0f;

            // Lightning strike flash at t ≈ 62s and t ≈ 70s
            bool lightning = (std::abs(t - 62.0f) < 0.15f) || (std::abs(t - 70.0f) < 0.12f);
            if (lightning) {
                m_camShake = 1.0f;
                p.fillRect(bounds, QColor(240, 245, 255, static_cast<int>(240 * alpha)));
            } else {
                QLinearGradient sky(0, 0, 0, bounds.height());
                sky.setColorAt(0.0f, QColor(10, 12, 18, static_cast<int>(255 * alpha)));
                sky.setColorAt(1.0f, QColor(20, 25, 35, static_cast<int>(255 * alpha)));
                p.fillRect(bounds, sky);
            }

            // Hollowmere Manor Gothic Towers Silhouette
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(5, 6, 8, static_cast<int>(255 * alpha)));

            float mx = bounds.width() * 0.5f + offset.x();
            float my = bounds.height() * 0.6f + offset.y();

            // Main central keep
            p.drawRect(static_cast<int>(mx - 140), static_cast<int>(my - 80), 280, 160);
            // Left Spire
            QPolygonF spireL;
            spireL << QPointF(mx - 180, my + 80) << QPointF(mx - 120, my + 80) << QPointF(mx - 150, my - 160);
            p.drawPolygon(spireL);
            // Right Spire
            QPolygonF spireR;
            spireR << QPointF(mx + 120, my + 80) << QPointF(mx + 180, my + 80) << QPointF(mx + 150, my - 160);
            p.drawPolygon(spireR);

            // Wrought iron gate in foreground
            p.setPen(QPen(QColor(8, 8, 12, static_cast<int>(255 * alpha)), 4.0f));
            for (int g = -10; g <= 10; ++g) {
                float gx = mx + g * 35;
                p.drawLine(QPointF(gx, bounds.height()), QPointF(gx, bounds.height() - 160));
                // Spike tip
                p.drawLine(QPointF(gx - 8, bounds.height() - 145), QPointF(gx, bounds.height() - 165));
                p.drawLine(QPointF(gx + 8, bounds.height() - 145), QPointF(gx, bounds.height() - 165));
            }
        }
    });

    // 5. Scene 5: Psychological Title Card (75s - 80s)
    addLayer({
        "TitleCard",
        4,
        [](QPainter& p, const QRect& bounds, float t, float, const QPointF&, float) {
            if (t < 74.5f) return;
            p.fillRect(bounds, Qt::black);

            float alpha = std::min(1.0f, (t - 74.5f) / 1.5f);
            if (t > 78.5f) alpha = std::max(0.0f, (80.0f - t) / 1.5f);

            QFont titleFont("Georgia", 38, QFont::Bold);
            titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 8.0);
            p.setFont(titleFont);

            int cx = bounds.width() / 2;
            int cy = bounds.height() / 2;

            // Chromatic glitch aberration
            p.setPen(QColor(220, 40, 40, static_cast<int>(120 * alpha)));
            p.drawText(QRect(cx - 302, cy - 40, 600, 80), Qt::AlignCenter, "H O L L O W M E R E");

            p.setPen(QColor(40, 180, 220, static_cast<int>(120 * alpha)));
            p.drawText(QRect(cx - 298, cy - 40, 600, 80), Qt::AlignCenter, "H O L L O W M E R E");

            p.setPen(QColor(240, 240, 245, static_cast<int>(255 * alpha)));
            p.drawText(QRect(cx - 300, cy - 40, 600, 80), Qt::AlignCenter, "H O L L O W M E R E");
        }
    });

    // Audio Cues & Narrative Voice / Subtitles
    addSubtitle(2.0f, 6.0f, "Mae", "Nora... keep the light close. The dark isn't empty anymore.", QColor(220, 160, 160));
    addSubtitle(19.0f, 7.0f, "Dr. Crane (Letter)", "'The children you could not save did not burn. They wait for you at Hollowmere.'", QColor(200, 200, 220));
    addSubtitle(42.0f, 6.0f, "Nora", "Seven years... and I still hear their crying in the floorboards.", QColor(240, 240, 240));
    addSubtitle(62.0f, 4.0f, "[Lightning cracks overhead]", "", QColor(180, 180, 180));
    addSubtitle(70.5f, 4.0f, "[Heavy wrought-iron gate slams shut behind you]", "", QColor(220, 80, 80));

    // Audio triggers
    if (m_audio) {
        addAudioCue(0.5f, [this]() { m_audio->playIntroScore(); });
        addAudioCue(62.0f, [this]() { m_audio->playJumpscareStinger(); });
        addAudioCue(70.5f, [this]() { m_audio->playDoorSlam(Vec3(0, 0, -2)); });
    }
}

} // namespace hm
