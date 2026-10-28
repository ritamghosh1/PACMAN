#include "Cutscenes.h"
#include "CineArt.h"
#include "CutsceneKit.h"
#include "audio/AudioEngine.h"
#include "audio/CineSfx.h"
#include <QFont>
#include <QLinearGradient>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

namespace hm {

using namespace cineart;
using namespace scenekit;
using cinesfx::Bank;

namespace {

// ---- E1: dawn at the open gate -------------------------------------------------------------
void drawDawn(QPainter& p, const ShotContext& c) {
    float t = c.t;
    float rise = smooth(t / 30.0f);
    sky(p, mix(QColor(16, 22, 46), QColor(86, 128, 188), rise), mix(QColor(70, 46, 66), QColor(255, 206, 130), rise), 700);
    glow(p, QPointF(800, 700 - 170 * rise), 900, QColor(255, 205, 130), 0.55f * rise);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 240, 205, static_cast<int>(255 * rise)));
    p.drawEllipse(QPointF(800, 700 - 170 * rise), 64, 64);
    clouds(p, t, mix(QColor(120, 90, 110), QColor(255, 214, 170), rise), 0.40f, 200, 220);
    // The manor goes dark window by window as the children leave it
    pines(p, t, 730, 180, mix(QColor(8, 10, 14), QColor(30, 40, 50), rise), 0, 7);
    manor(p, t, QPointF(1180, 735), 0.55f, std::max(0.0f, 1.0f - smooth((t - 3.0f) / 22.0f)), mix(QColor(6, 7, 10), QColor(36, 40, 52), rise), 0.0f);
    QLinearGradient ground(0, 700, 0, H);
    ground.setColorAt(0.0, mix(QColor(10, 18, 20), QColor(52, 60, 50), rise));
    ground.setColorAt(1.0, QColor(6, 10, 10));
    p.fillRect(QRectF(-100, 700, W + 200, 300), ground);
    QPolygonF path;
    path << QPointF(780, 704) << QPointF(820, 704) << QPointF(1180, H + 20) << QPointF(420, H + 20);
    QLinearGradient pg(0, 700, 0, H);
    pg.setColorAt(0.0, QColor(120, 100, 80, static_cast<int>(100 + 120 * rise)));
    pg.setColorAt(1.0, QColor(40, 36, 30, 220));
    p.setBrush(pg);
    p.drawPolygon(path);
    fog(p, t, QColor(255, 225, 195), 0.20f + 0.2f * rise, 560, 790);
    // Light shafts
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    for (int i = 0; i < 4; ++i) {
        QPolygonF sh;
        float a = -0.5f + i * 0.34f;
        sh << QPointF(800, 700 - 170 * rise) << QPointF(800 + a * 1400 - 90, H) << QPointF(800 + a * 1400 + 90, H);
        QLinearGradient sg(800, 700, 800 + a * 1400, H);
        sg.setColorAt(0.0, QColor(255, 220, 150, static_cast<int>(46 * rise)));
        sg.setColorAt(1.0, QColor(255, 220, 150, 0));
        p.setBrush(sg);
        p.drawPolygon(sh);
    }
    p.restore();
    // Nora and Mae walk toward the sun hand in hand
    float walk = std::min(t, 36.0f);
    QPointF feet(790, 790 - walk * 2.8f);
    float s = std::max(0.36f, 0.86f - walk * 0.0140f);
    QColor sil = mix(QColor(5, 6, 8), QColor(40, 34, 36), rise * 0.4f);
    figureBack(p, feet, s, sil, std::sin(t * 1.8f) * 2.5f);
    child(p, QPointF(feet.x() + 74 * s, feet.y()), s * 1.55f, sil, QColor(190, 28, 40), std::sin(t * 2.0f) * 2.0f);
    p.setPen(QPen(sil, 5 * s));
    p.drawLine(QPointF(feet.x() + 34 * s, feet.y() - 100 * s), QPointF(feet.x() + 96 * s, feet.y() - 66 * s * 1.5f));
    // Souls rising: small warm lights, one for every child
    embers(p, t, QRectF(260, 60, 1080, 700), 22, 0.07f, QColor(255, 228, 160));
    // Gate standing open in the foreground
    gate(p, QRectF(-30, 380, W + 60, 520), 0.98f, QColor(3, 4, 5));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(10, 11, 14));
    p.drawRect(QRectF(-60, 300, 150, 700));
    p.drawRect(QRectF(W - 90, 300, 150, 700));
    p.fillRect(QRectF(-100, -100, W + 200, H + 200), QColor(255, 190, 120, static_cast<int>(26 * rise)));
}

// ---- E2: ash in the foyer, the keys --------------------------------------------------------
void drawAshHall(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(5, 4, 5));
    corridor(p, t, QPointF(800, 430), QColor(44, 38, 38), QColor(34, 28, 28));
    // Cold shaft from a broken skylight
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    QPolygonF shaft;
    shaft << QPointF(660, -40) << QPointF(940, -40) << QPointF(1080, H) << QPointF(520, H);
    QLinearGradient sg(0, 0, 0, H);
    sg.setColorAt(0.0, QColor(150, 180, 230, 80));
    sg.setColorAt(1.0, QColor(150, 180, 230, 18));
    p.setPen(Qt::NoPen);
    p.setBrush(sg);
    p.drawPolygon(shaft);
    p.restore();
    embers(p, t, QRectF(100, 300, 1400, 600), 30, 0.12f, QColor(255, 120, 40));
    // Falling ash
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 220; ++i) {
        float x = std::fmod(hash(i, 3) * W + std::sin(t * 0.7f + i) * 30, W);
        float y = std::fmod(hash(i, 4) * H + t * (28 + hash(i, 5) * 50), H);
        float r = 1.2f + hash(i, 6) * 2.6f;
        float inShaft = (x > 540 && x < 1060) ? 1.0f : 0.45f;
        p.setBrush(QColor(210, 210, 215, static_cast<int>(110 * inShaft * (0.4f + 0.6f * hash(i, 7)))));
        p.drawEllipse(QPointF(x, y), r, r);
    }
    // Nora with the warden's keys
    QPointF feet(800, 820);
    figureBack(p, feet, 0.95f, QColor(3, 3, 4), std::sin(t * 0.9f) * 2.0f);
    float lift = (c.beat >= 2) ? smooth(c.beatT / 1.6f) : 0.0f;
    QPointF ring(feet.x() + 46, feet.y() - 120 - 90 * lift);
    glow(p, ring, 120 + 80 * lift, QColor(255, 190, 90), 0.25f + 0.4f * lift);
    p.setPen(QPen(QColor(150, 150, 160), 3));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(ring, 11, 11);
    for (int k = 0; k < 5; ++k) {
        float a = 1.2f + k * 0.55f + std::sin(t * 2.4f + k) * 0.08f;
        QPointF e = ring + QPointF(std::cos(a) * 48, std::sin(a) * 48);
        p.setPen(QPen(QColor(120, 120, 130), 5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(ring + QPointF(std::cos(a) * 11, std::sin(a) * 11), e);
        p.drawEllipse(e, 6, 6);
    }
    p.fillRect(QRectF(-100, -100, W + 200, H + 200), QColor(20, 24, 40, 30));
}

// ---- E3: the endless drive -----------------------------------------------------------------
void drawLoopDrive(QPainter& p, const ShotContext& c) {
    ShotContext c2 = c;
    c2.t = c.t + 15.0f;      // the roadside sign is already waiting
    drawDrive(p, c2);
    fog(p, c.t, QColor(120, 140, 160), 0.18f, 330, 560);
    // Rear-view mirror: the manor, always the same distance behind
    p.save();
    QRectF mir(W * 0.5f - 120, 38, 240, 54);
    p.setClipRect(mir);
    p.fillRect(mir, QColor(20, 24, 30));
    manor(p, c.t, QPointF(W * 0.5f, 98), 0.085f, 1.0f, QColor(4, 5, 7), 0.0f);
    glow(p, QPointF(W * 0.5f, 70), 70, QColor(255, 170, 80), 0.18f);
    p.restore();
    // Mae, silent in the passenger seat, ribbon catching the dash light
    child(p, QPointF(1340, 960), 2.0f, QColor(3, 3, 4), QColor(190, 28, 40), std::sin(c.t * 0.8f) * 3.0f);
    float pulse = 0.5f + 0.5f * std::sin(c.t * 2.9f);
    glow(p, QPointF(800, 780), 220, QColor(255, 150, 60), 0.18f + 0.1f * pulse);
}

// ---- E4: candles at the altar --------------------------------------------------------------
void drawChapel(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(8, 5, 6));
    // Tall arched window, first light behind it
    float dawn = smooth((t - 2.0f) / 26.0f);
    QPainterPath arch;
    arch.moveTo(560, 600);
    arch.lineTo(560, 220);
    arch.arcTo(QRectF(560, 20, 480, 400), 180, -180);
    arch.lineTo(1040, 600);
    arch.closeSubpath();
    QLinearGradient wg(0, 40, 0, 600);
    wg.setColorAt(0.0, mix(QColor(20, 30, 60), QColor(120, 160, 220), dawn));
    wg.setColorAt(1.0, mix(QColor(40, 30, 40), QColor(255, 210, 150), dawn));
    p.setPen(Qt::NoPen);
    p.setBrush(wg);
    p.drawPath(arch);
    p.setPen(QPen(QColor(6, 5, 7), 10));
    p.drawLine(QPointF(800, 20), QPointF(800, 600));
    p.drawLine(QPointF(560, 300), QPointF(1040, 300));
    p.drawLine(QPointF(560, 450), QPointF(1040, 450));
    p.setPen(QPen(QColor(6, 5, 7), 18));
    p.setBrush(Qt::NoBrush);
    p.drawPath(arch);
    glow(p, QPointF(800, 330), 700, QColor(190, 205, 240), 0.22f + 0.3f * dawn);
    // Light falling to the floor
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    QPolygonF sh;
    sh << QPointF(560, 600) << QPointF(1040, 600) << QPointF(1300, H) << QPointF(300, H);
    QLinearGradient sg(0, 600, 0, H);
    sg.setColorAt(0.0, QColor(200, 210, 240, static_cast<int>(60 + 60 * dawn)));
    sg.setColorAt(1.0, QColor(200, 210, 240, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(sg);
    p.drawPolygon(sh);
    p.restore();
    // Floor and altar
    QLinearGradient fl(0, 600, 0, H);
    fl.setColorAt(0.0, QColor(24, 16, 14));
    fl.setColorAt(1.0, QColor(10, 7, 7));
    p.setPen(Qt::NoPen);
    p.fillRect(QRectF(-100, 600, W + 200, 400), fl);
    p.setBrush(QColor(12, 9, 10));
    p.drawRect(QRectF(640, 540, 320, 78));
    p.setBrush(QColor(40, 30, 28));
    p.drawRect(QRectF(630, 532, 340, 14));
    // Candles: rows lit one by one as each child is let go
    int total = 0, lit = static_cast<int>(smooth((t - 3.0f) / 22.0f) * 40.0f) + (c.beat >= 1 ? 6 : 0);
    for (int row = 0; row < 5; ++row) {
        float s = 0.38f + 0.16f * row;
        int n = 4 + row * 2;
        float y = 640 + row * 52.0f;
        for (int i = 0; i < n; ++i) {
            float x = 800 + (i - (n - 1) * 0.5f) * (74.0f + row * 22.0f);
            if (total++ < lit) candle(p, QPointF(x, y), t, s, total);
            else { p.setBrush(QColor(60, 54, 44)); p.drawRoundedRect(QRectF(x - 8 * s, y - 70 * s, 16 * s, 70 * s), 3 * s, 3 * s); }
        }
    }
    // The children: pale shapes who dissolve upward into the candlelight
    float leave = (c.beat >= 1) ? smooth(c.beatT / 7.0f) : 0.0f;
    p.save();
    for (int i = 0; i < 7; ++i) {
        float x = 520 + i * 90.0f + (i > 3 ? 240.0f : 0.0f);
        float a = (1.0f - leave) * 0.32f * smooth((t - 1.5f) / 3.0f);
        p.setOpacity(a);
        child(p, QPointF(x, 640 - leave * 220.0f * (0.6f + hash(i, 4))), 0.92f, QColor(150, 160, 190), QColor(190, 28, 40), std::sin(t * 1.3f + i) * 2.0f);
    }
    p.restore();
    embers(p, t, QRectF(380, 120, 840, 560), static_cast<int>(6 + 28 * leave), 0.10f, QColor(255, 220, 150));
    // Nora kneeling, singing to them; Mae slips away into the light on the right
    bust(p, QPointF(800, 700), 0.72f, QColor(4, 4, 6), std::sin(t * 0.9f) * 2.0f);
    float mae = (c.beat >= 2) ? smooth(c.beatT / 9.0f) : 0.0f;
    p.save();
    p.setOpacity(c.beat >= 2 ? 1.0f : 0.0f);
    glow(p, QPointF(1420, 560), 360, QColor(255, 230, 190), 0.35f * mae + 0.05f);
    child(p, QPointF(1180 + 250 * mae, 780 - 40 * mae), 1.2f - 0.25f * mae, QColor(5, 5, 7), QColor(190, 28, 40), std::sin(t * 2.6f) * 2.0f);
    p.restore();
}

// ---- E5: the archive drawer ----------------------------------------------------------------
void drawArchive(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(8, 6, 4));
    // Shelves of folders
    p.setPen(Qt::NoPen);
    for (int row = 0; row < 5; ++row) {
        float y = 40 + row * 100.0f;
        p.setBrush(QColor(22, 14, 8));
        p.drawRect(QRectF(-20, y + 90, W + 40, 10));
        for (int i = 0; i < 44; ++i) {
            float h = 60 + hash(i + row * 50, 2) * 30;
            int g = 24 + static_cast<int>(hash(i + row * 50, 3) * 30);
            p.setBrush(QColor(g + 14, g, g - 8));
            p.drawRect(QRectF(i * 38.0f, y + 90 - h, 32, h));
        }
    }
    float flick = 0.88f + 0.12f * vnoise(t * 6.0f, 2);
    glow(p, QPointF(800, 260), 1000, QColor(255, 175, 90), 0.5f * flick);
    // Desk and open drawer
    QLinearGradient desk(0, 560, 0, H);
    desk.setColorAt(0.0, QColor(66, 40, 22));
    desk.setColorAt(1.0, QColor(22, 12, 8));
    p.setBrush(desk);
    p.drawRect(QRectF(-100, 560, W + 200, 400));
    p.setBrush(QColor(30, 18, 10));
    p.drawRect(QRectF(560, 600, 480, 120));
    p.setBrush(QColor(8, 5, 3));
    p.drawRect(QRectF(580, 620, 440, 90));
    // The chart
    p.save();
    p.translate(800, 380 + (1.0f - smooth(t / 3.0f)) * 160);
    p.rotate(-2.5f);
    p.setPen(QPen(QColor(110, 90, 60), 2));
    p.setBrush(QColor(226, 212, 176));
    p.drawRect(QRectF(-290, -250, 580, 500));
    p.setPen(QColor(48, 38, 28));
    QFont f("Courier New");
    f.setPixelSize(26);
    f.setBold(true);
    p.setFont(f);
    static const char* rows[] = {"ST. ALDOUS HOME FOR CHILDREN", "PATIENT RECORD  --  QUIET PROG.", "No.   404", "NAME  HALE, NORA", "ADMITTED   1991", "ATTENDING  DR. S. CRANE", "STATUS     TREATED.  SEALED."};
    float typed = (t - 3.0f) * 26.0f;
    int y = -190;
    for (const char* r : rows) {
        QString line = QString::fromUtf8(r);
        int n = std::clamp(static_cast<int>(typed), 0, static_cast<int>(line.size()));
        p.drawText(QPointF(-260, y), line.left(n));
        typed -= line.size() + 6;
        y += 52;
    }
    if (c.beat >= 1) {       // SEALED stamp lands
        float k = smooth(c.beatT / 0.25f);
        p.save();
        p.translate(120, 150);
        p.rotate(-14);
        p.scale(1.0f + (1.0f - k) * 0.8f, 1.0f + (1.0f - k) * 0.8f);
        p.setOpacity(k);
        p.setPen(QPen(QColor(170, 28, 34), 7));
        p.setBrush(Qt::NoBrush);
        p.drawRect(QRectF(-130, -42, 260, 84));
        QFont sf("Courier New");
        sf.setPixelSize(54);
        sf.setBold(true);
        p.setFont(sf);
        p.setPen(QColor(170, 28, 34));
        p.drawText(QRectF(-130, -42, 260, 84), Qt::AlignCenter, "SEALED");
        p.restore();
    }
    p.restore();
    // Nora's hand holds the corner of the page
    hand(p, QPointF(1210, 760), 1.45f, -28, QColor(8, 6, 6), 0.55f);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 60; ++i) {
        float x = 300 + std::fmod(hash(i, 51) * 1000 + t * (3 + hash(i, 52) * 6), 1000);
        float y2 = 100 + hash(i, 53) * 520 + std::sin(t * 0.4f + i) * 12;
        p.setBrush(QColor(255, 225, 170, static_cast<int>(20 + 50 * hash(i, 54))));
        p.drawEllipse(QPointF(x, y2), 1.5, 1.5);
    }
}

// ---- Credits roll ---------------------------------------------------------------------------
void drawCredits(QPainter& p, const ShotContext& c, const QString& title, const QColor& accent) {
    float t = c.t;
    fillAll(p, QColor(3, 3, 5));
    fog(p, t, mix(accent, QColor(0, 0, 0), 0.7f), 0.30f, 250, 650);
    embers(p, t, QRectF(0, 0, W, H), 22, 0.08f, mix(accent, QColor(255, 255, 255), 0.3f));
    // Title card first
    float card = t < 5.5f ? smooth(t / 1.2f) * (1.0f - smooth((t - 4.2f) / 1.2f)) : 0.0f;
    if (card > 0.0f) {
        QFont tf("Georgia");
        tf.setPixelSize(30);
        tf.setLetterSpacing(QFont::AbsoluteSpacing, 8.0);
        p.setFont(tf);
        p.setPen(QColor(190, 185, 180, static_cast<int>(200 * card)));
        p.drawText(QRectF(0, 330, W, 50), Qt::AlignHCenter, "THE END");
        QFont f("Georgia");
        f.setPixelSize(54);
        f.setBold(true);
        f.setLetterSpacing(QFont::AbsoluteSpacing, 4.0);
        p.setFont(f);
        int n = std::clamp(static_cast<int>((t - 0.8f) / 0.05f), 0, static_cast<int>(title.size()));
        p.setPen(QColor(accent.red(), accent.green(), accent.blue(), static_cast<int>(255 * card)));
        p.drawText(QRectF(0, 400, W, 80), Qt::AlignHCenter, title.left(n));
    }
    struct Entry { const char* head; const char* body; };
    static const Entry credits[] = {
        {"HOLLOWMERE", "A psychological horror in three dimensions"},
        {"CONCEPT & DIRECTION", "Computer Graphics Systems Lab"},
        {"3D ENGINE", "OpenGL 3.3 core profile  -  Qt 5"},
        {"PROCEDURAL SOUND DESIGN", "Rain, fire, thunder, music box, motifs and typewriter ticks\nall synthesised in code  -  there is no voice acting"},
        {"GHOST AI", "Sight, sound and noise-bus perception"},
        {"STORY", "The Quiet Programme at St. Aldous Home"},
        {"IN MEMORY OF", "Ada  -  Eli  -  and the ten who never walked out"},
        {"THANK YOU FOR PLAYING", ""},
    };
    float scroll = H + 40 - (t - 5.0f) * 56.0f;
    float y = scroll;
    for (const auto& e : credits) {
        QString body = QString::fromUtf8(e.body);
        int lines = body.isEmpty() ? 0 : body.count('\n') + 1;
        if (y > -140 && y < H + 60) {
            QFont hf("Georgia");
            hf.setPixelSize(32);
            hf.setBold(true);
            hf.setLetterSpacing(QFont::AbsoluteSpacing, 3.0);
            p.setFont(hf);
            p.setPen(QColor(accent.red(), accent.green(), accent.blue(), 235));
            p.drawText(QRectF(0, y, W, 44), Qt::AlignHCenter, QString::fromUtf8(e.head));
            QFont bf("Georgia");
            bf.setPixelSize(24);
            bf.setItalic(true);
            p.setFont(bf);
            p.setPen(QColor(205, 200, 195, 225));
            p.drawText(QRectF(0, y + 48, W, 30.0 * std::max(1, lines) + 10), Qt::AlignHCenter | Qt::AlignTop, body);
        }
        y += 120 + 32 * lines;
    }
}

QStringList sentences(const QString& text) {
    return text.split(QRegularExpression("(?<=[.!?])\\s+"), Qt::SkipEmptyParts);
}

} // namespace

std::unique_ptr<Cinematic> makeEndingCinematic(EndingType ending, AudioEngine* audio, bool creditsOnly) {
    const Bank& B = Bank::get();
    auto cine = std::make_unique<Cinematic>(audio);
    const int idx = static_cast<int>(ending);
    const QString epilogue = QString::fromStdString(EndingResolver::getEndingEpilogue(ending));
    const QString title = QString::fromStdString(EndingResolver::getEndingTitle(ending));

    Shot s;
    s.lead = 3.0f;
    s.zoom0 = 1.0f; s.zoom1 = 1.18f; s.camTime = 40.0f;
    s.fadeIn = 2.0f; s.fadeOut = 2.0f;
    s.outro = 2.0f;
    QColor accent;
    for (const QString& sent : sentences(epilogue)) s.lines.push_back({Speaker::Narrator, sent});

    switch (ending) {
    case EndingType::E1_Release:
        s.id = "dawn";
        s.draw = drawDawn;
        accent = QColor(240, 205, 140);
        s.onEnter = [&B, audio, idx](Cinematic& c) {
            bed(c, BedWind, B.wind, 0.30f, 3.0f);
            bed(c, BedPad, B.padLow, 0.30f, 4.0f);
            if (audio) audio->playEndingScore(idx);
            sfx(c, B.creak, 0.5f);
        };
        s.onLine = [&B](Cinematic& c, int i) { if (i == 1) sfx(c, B.windowOn, 0.5f); if (i == 2) bed(c, BedBox, B.boxClean, 0.4f, 1.5f); };
        break;
    case EndingType::E2_Consumed:
        s.id = "ash";
        s.draw = drawAshHall;
        accent = QColor(205, 100, 90);
        s.onEnter = [&B, audio, idx](Cinematic& c) {
            bed(c, BedFire, B.fire, 0.22f, 3.0f);
            bed(c, BedPad, B.padTense, 0.45f, 3.0f);
            if (audio) audio->playEndingScore(idx);
        };
        s.onLine = [&B](Cinematic& c, int i) { if (i == 2) { sfx(c, B.lock, 1.0f); sfx(c, B.impact, 0.5f); c.shake(0.4f); } };
        break;
    case EndingType::E3_Loop:
        s.id = "loop";
        s.draw = drawLoopDrive;
        accent = QColor(150, 165, 205);
        s.onEnter = [&B, audio, idx](Cinematic& c) {
            bed(c, BedRain, B.rain, 0.35f, 2.0f);
            bed(c, BedCar, B.car, 0.5f, 2.0f);
            bed(c, BedBox, B.boxWarped, 0.35f, 3.0f);
            if (audio) audio->playEndingScore(idx);
        };
        s.onLine = [&B](Cinematic& c, int i) { if (i == 1) sfx(c, B.thunderB, 0.6f); if (i >= 2) sfx(c, B.knock, 0.4f); };
        break;
    case EndingType::E4_Martyr:
        s.id = "chapel";
        s.draw = drawChapel;
        accent = QColor(236, 178, 96);
        s.onEnter = [&B, audio, idx](Cinematic& c) {
            bed(c, BedExtra, B.graveyard, 0.30f, 3.0f);
            bed(c, BedBox, B.boxClean, 0.40f, 2.0f);
            if (audio) audio->playEndingScore(idx);
        };
        s.onLine = [&B](Cinematic& c, int i) { if (i == 1) { sfx(c, B.windowOn, 0.7f); bedStop(c, BedBox, 4.0f); } if (i == 2) sfx(c, B.lock, 0.6f); };
        break;
    case EndingType::E5_Revelation:
        s.id = "archive";
        s.draw = drawArchive;
        accent = QColor(240, 220, 130);
        s.onEnter = [&B, audio, idx](Cinematic& c) {
            bed(c, BedPad, B.padTense, 0.35f, 3.0f);
            sfx(c, B.clock, 0.5f);
            if (audio) audio->playEndingScore(idx);
        };
        s.onLine = [&B](Cinematic& c, int i) { if (i == 1) { sfx(c, B.rustle, 0.8f); sfx(c, B.thud, 0.9f); c.shake(0.6f); } };
        break;
    }
    if (!creditsOnly) cine->addShot(std::move(s));

    Shot credits;
    credits.id = "credits";
    credits.draw = [title, accent](QPainter& p, const ShotContext& c) { drawCredits(p, c, title, accent); };
    credits.autoDuration = 5.0f + 8 * 5.0f + 6.0f;
    credits.fadeIn = 2.0f;
    credits.fadeOut = 2.5f;
    credits.zoom0 = credits.zoom1 = 1.0f;
    credits.onEnter = [&B](Cinematic& c) { bedStop(c, BedFire, 3.0f); bedStop(c, BedCar, 3.0f); bed(c, BedPad, B.padLow, 0.3f, 3.0f); };
    cine->addShot(std::move(credits));
    return cine;
}

} // namespace hm
