#include "Cutscenes.h"
#include "CineArt.h"
#include "CutsceneKit.h"
#include "audio/AudioEngine.h"
#include "audio/CineSfx.h"
#include <QLinearGradient>
#include <QPainterPath>
#include <QRadialGradient>
#include <cmath>

namespace hm {

using namespace cineart;
using namespace scenekit;
using cinesfx::Bank;

namespace {

// ---- Memory 1: Ada, the girl who was punished for crying -------------------------------------
void drawAda(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(8, 8, 12));
    // Moonlit window and rain
    QRectF win(1020, 90, 360, 520);
    p.fillRect(win, QLinearGradient(win.topLeft(), win.bottomLeft()));
    QLinearGradient sky(win.topLeft(), win.bottomLeft());
    sky.setColorAt(0, QColor(40, 54, 86));
    sky.setColorAt(1, QColor(18, 24, 40));
    p.fillRect(win, sky);
    p.save();
    p.setClipRect(win);
    rain(p, t, 0.5f, 10.0f, 700.0f, 0.5f);
    rainOnGlass(p, t, win, 40, 1.0f);
    p.restore();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(10, 10, 12));
    p.drawRect(QRectF(win.center().x() - 8, win.top(), 16, win.height()));
    p.drawRect(QRectF(win.left(), win.center().y() - 8, win.width(), 16));
    glow(p, QPointF(win.center().x(), win.bottom()), 700, QColor(120, 150, 210), 0.20f);
    // Bookshelves: rows of spines
    for (int s = 0; s < 3; ++s) {
        float x = 60 + s * 300.0f;
        p.setBrush(QColor(22, 14, 10));
        p.drawRect(QRectF(x, 120, 260, 520));
        for (int r = 0; r < 5; ++r)
            for (int i = 0; i < 12; ++i) {
                int g = 30 + static_cast<int>(hash(i + r * 12 + s * 70, 3) * 40);
                p.setBrush(QColor(g + 20, g, g - 10));
                p.drawRect(QRectF(x + 8 + i * 20.5f, 130 + r * 100 + (1 - hash(i + r, 4)) * 14, 17, 84));
            }
    }
    p.setBrush(QColor(14, 10, 8));
    p.drawRect(QRectF(-100, 640, W + 200, 400));
    // Ada in the corner, knees up, ribbon in her hair, tears with no sound
    QPointF feet(780, 790);
    child(p, feet, 2.3f, QColor(6, 6, 9), QColor(190, 28, 40), std::sin(t * 2.2f) * 3.0f);
    for (int i = 0; i < 7; ++i) {
        float life = std::fmod(t * 0.7f + i * 0.17f, 1.0f);
        QPointF tp(feet.x() - 4 + (i % 2 ? 14 : -10), feet.y() - 285 * 0.93f + life * 150);
        glow(p, tp, 14, QColor(170, 200, 255), (1 - life) * 0.9f);
    }
    // A mouth shaped around a lullaby with no sound: three pale notes drifting up and fading
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    for (int i = 0; i < 3; ++i) {
        float life = std::fmod(t * 0.25f + i * 0.33f, 1.0f);
        QPointF q(feet.x() + 50 + i * 30, feet.y() - 330 - life * 120);
        glow(p, q, 18, QColor(255, 220, 160), std::sin(life * 3.14159f) * 0.5f);
    }
    p.restore();
    glow(p, QPointF(260, 560), 520, QColor(255, 170, 90), 0.20f);
    candle(p, QPointF(260, 640), t, 1.4f, 3);
}

// ---- Memory 2: the garden of small graves ----------------------------------------------------
void drawGarden(QPainter& p, const ShotContext& c) {
    float t = c.t;
    sky(p, QColor(8, 18, 20), QColor(22, 40, 36), 700);
    glow(p, QPointF(1180, 160), 420, QColor(150, 200, 190), 0.22f);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(210, 230, 220, 190));
    p.drawEllipse(QPointF(1180, 160), 46, 46);
    // Glass dome ribs
    p.setPen(QPen(QColor(10, 24, 22), 8));
    p.setBrush(Qt::NoBrush);
    for (int i = 0; i < 5; ++i) p.drawArc(QRectF(100 + i * 60.0f, -300 + i * 30.0f, 1400 - i * 120.0f, 1000 - i * 60.0f), 0, 180 * 16);
    pines(p, t, 700, 160, QColor(4, 12, 10), 0, 9);
    QLinearGradient ground(0, 640, 0, H);
    ground.setColorAt(0, QColor(10, 22, 18));
    ground.setColorAt(1, QColor(4, 10, 8));
    p.setPen(Qt::NoPen);
    p.fillRect(QRectF(-100, 640, W + 200, 400), ground);
    // Rows of flowerbeds: each has a small marker with a name
    for (int row = 0; row < 4; ++row)
        for (int i = 0; i < 9; ++i) {
            float s = 0.5f + row * 0.25f;
            float x = 140 + i * 160.0f * (0.6f + row * 0.15f) + row * 30;
            float y = 690 + row * 60.0f;
            p.setBrush(QColor(8, 16, 12));
            p.drawEllipse(QPointF(x, y + 6 * s), 52 * s, 14 * s);
            p.setBrush(QColor(60, 70, 66));
            p.drawRoundedRect(QRectF(x - 14 * s, y - 62 * s, 28 * s, 62 * s), 8 * s, 8 * s);
            p.setBrush(QColor(110, 150, 120, 140));
            p.drawEllipse(QPointF(x, y - 20 * s), 6 * s, 6 * s);
        }
    // Mr. Pell, far off between the beds, shears in hand, face a blank oval
    float px = 400 + std::fmod(t * 14.0f, 700.0f);
    p.setBrush(QColor(3, 6, 5));
    p.drawRoundedRect(QRectF(px - 22, 520, 44, 110), 14, 14);
    p.drawEllipse(QPointF(px, 506), 18, 22);
    p.setPen(QPen(QColor(3, 6, 5), 5));
    p.drawLine(QPointF(px + 20, 560), QPointF(px + 56, 520));
    p.setPen(QPen(QColor(200, 210, 210, 200), 3));
    float snip = std::fabs(std::sin(t * 6.0f)) * 12.0f;
    p.drawLine(QPointF(px + 56, 520), QPointF(px + 80, 506 - snip));
    p.drawLine(QPointF(px + 56, 520), QPointF(px + 80, 506 + snip));
    fog(p, t, QColor(120, 170, 150), 0.22f, 560, 780);
}

// ---- Memory 3: the Chorus Ward ---------------------------------------------------------------
void drawChorus(QPainter& p, const ShotContext& c) {
    float t = c.t;
    fillAll(p, QColor(6, 8, 12));
    // Boiler pipes and a glowing gauge
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 6; ++i) {
        p.setBrush(QColor(22 + i * 3, 26 + i * 3, 32 + i * 3));
        p.drawRect(QRectF(-20, 90 + i * 66.0f, W + 40, 28));
        p.setBrush(QColor(8, 10, 14));
        p.drawRect(QRectF(150 + i * 230.0f, 60, 34, 560));
    }
    glow(p, QPointF(1300, 260), 260, QColor(255, 120, 50), 0.35f + 0.1f * std::sin(t * 3));
    p.setBrush(QColor(255, 160, 60, 200));
    p.drawEllipse(QPointF(1300, 260), 24, 24);
    // Flooded floor with reflections
    QLinearGradient water(0, 640, 0, H);
    water.setColorAt(0, QColor(14, 22, 34));
    water.setColorAt(1, QColor(6, 10, 16));
    p.fillRect(QRectF(-100, 640, W + 200, 400), water);
    p.setPen(QPen(QColor(120, 160, 210, 40), 2));
    for (int i = 0; i < 12; ++i) {
        float y = 660 + i * 22.0f;
        float off = std::sin(t * 1.4f + i) * 12.0f;
        p.drawLine(QPointF(300 + off, y), QPointF(1300 + off, y));
    }
    // Three children whose throats are joined by one glowing band; the chord shimmers
    for (int i = 0; i < 3; ++i) {
        float x = 560 + i * 240.0f;
        float sway = std::sin(t * 1.3f + i) * 3.0f;
        child(p, QPointF(x, 840), 1.9f, QColor(5, 6, 9), QColor(120, 120, 140), sway);
    }
    float pulse = 0.55f + 0.45f * std::sin(t * 4.0f);
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.setPen(QPen(QColor(255, 200, 120, static_cast<int>(200 * pulse)), 6, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(560, 590), QPointF(1040, 590));
    for (int i = 0; i < 3; ++i) glow(p, QPointF(560 + i * 240.0f, 590), 90, QColor(255, 190, 110), 0.5f * pulse);
    // Sound rings radiating out of the joined throats
    for (int k = 0; k < 4; ++k) {
        float life = std::fmod(t * 0.4f + k * 0.25f, 1.0f);
        p.setPen(QPen(QColor(255, 210, 150, static_cast<int>(120 * (1 - life))), 3));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(800, 590), 80 + life * 420, 30 + life * 150);
    }
    p.restore();
    fog(p, t, QColor(160, 170, 190), 0.20f, 480, 760);
    if (c.t < 0.0f) p.fillRect(QRectF(0, 0, 1, 1), Qt::black);
}

// ---- Memory 4: Eli behind the door -----------------------------------------------------------
void drawEli(QPainter& p, const ShotContext& c) {
    ShotContext c2 = c;
    c2.t = c.t + 12.0f;       // the door is already hot
    c2.beat = c.beat >= 1 ? 4 : 2;
    c2.beatT = c.beatT;
    drawDoorScene(p, c2);
    // Nora, nine years old, small and dark in the foreground with her hand on the lock
    child(p, QPointF(520, 960), 3.0f, QColor(3, 3, 4), QColor(190, 28, 40), std::sin(c.t * 1.8f) * 2.0f);
    // A tiny hand-shape glow low on the door: Eli pressing his palm to the wood
    float pulse = 0.5f + 0.5f * std::sin(c.t * 5.0f);
    glow(p, QPointF(800, 560), 60, QColor(255, 140, 60), 0.35f * pulse);
}

} // namespace

std::unique_ptr<Cinematic> makeMemoryCinematic(int sigil, AudioEngine* audio) {
    const Bank& B = Bank::get();
    auto cine = std::make_unique<Cinematic>(audio);
    Shot s;
    s.lead = 1.8f;
    s.fadeIn = 1.4f;
    s.fadeOut = 1.4f;
    s.outro = 1.2f;
    s.zoom0 = 1.0f; s.zoom1 = 1.14f; s.camTime = 22.0f;
    switch (sigil) {
    case 1:
        s.id = "memory-ada";
        s.draw = drawAda;
        s.lines = {
            {Speaker::Narrator, U("MEMORY ONE: ADA")},
            {Speaker::Narrator, U("Ada was eight. She cried every night, so Dr. Crane sealed her voice and her tear ducts.")},
            {Speaker::Ada,      U("I only wanted my mama.")},
            {Speaker::Nora,     U("I heard her through the vents. I told myself it was the wind.")},
        };
        s.onEnter = [&B](Cinematic& c) { music(c, "music_library", 0.55f, 2.0f); bed(c, BedRain, B.rain, 0.06f, 1.5f); bed(c, BedBox, B.boxWarped, 0.25f, 1.5f); };
        break;
    case 2:
        s.id = "memory-garden";
        s.draw = drawGarden;
        s.lines = {
            {Speaker::Narrator, U("MEMORY TWO: THE GARDEN")},
            {Speaker::Narrator, U("Mr. Pell kept the garden. Children who would not stay quiet were “pruned.”")},
            {Speaker::Nora,     U("Every flowerbed has a name pushed into the soil. These are graves.")},
            {Speaker::Crane,    U("A PLANT MUST BE CUT DOWN TO BEAR FRUIT.")},
        };
        s.onEnter = [&B](Cinematic& c) { music(c, "music_garden", 0.55f, 2.0f); bed(c, BedExtra, B.graveyard, 0.25f, 1.5f); };
        s.onLine = [&B](Cinematic& c, int i) { if (i >= 1) sfx(c, B.clock, 0.25f); };
        break;
    case 3:
        s.id = "memory-chorus";
        s.draw = drawChorus;
        s.lines = {
            {Speaker::Narrator, U("MEMORY THREE: THE CHORUS WARD")},
            {Speaker::Narrator, U("Three children were taken below. Their throats were joined so they could only sing one chord.")},
            {Speaker::Nora,     U("It cracked the walls. The staff called it therapy.")},
            {Speaker::Crane,    U("HARMONY IS OBEDIENCE.")},
        };
        s.onEnter = [](Cinematic& c) {
            music(c, "music_cellar", 0.6f, 2.0f);
            if (const AudioClip* v = AssetAudio::instance().find("pd_voices")) if (c.audio()) c.audio()->playClip(v, 0.45f);
        };
        s.onLine = [&B](Cinematic& c, int i) { if (i == 3) { sfx(c, B.impact, 0.5f); c.shake(0.5f); } };
        break;
    default:
        s.id = "memory-eli";
        s.draw = drawEli;
        s.lines = {
            {Speaker::Narrator, U("MEMORY FOUR: ELI")},
            {Speaker::Narrator, U("Eli was five. Dr. Crane gave him a porcelain mask to keep him calm. He never stopped playing hide-and-seek.")},
            {Speaker::Eli,      U("Nora? The door is hot. Please open it.")},
            {Speaker::Nora,     U("I locked it. I was nine. I ran, and I have not stopped running since.")},
            {Speaker::Nora,     U("I'm so sorry, Eli. I'm going back for you.")},
        };
        s.onEnter = [&B](Cinematic& c) { music(c, "music_nursery", 0.55f, 2.0f); bed(c, BedFire, B.fire, 0.35f, 1.5f); };
        s.onLine = [&B](Cinematic& c, int i) { if (i == 2) sfx(c, B.knock, 0.9f); if (i == 3) { sfx(c, B.lock, 1.0f); sfx(c, B.thud, 0.8f); c.shake(0.8f); } };
        break;
    }
    // After the memory, say plainly what it unlocked
    static const char* wrap[5] = {"", "Sigil 1 of 4 recovered. Ada can rest if you hum to her. Three memories remain.", "Sigil 2 of 4 recovered. Mr. Pell freezes under your lantern. Two memories remain.", "Sigil 3 of 4 recovered. The Warden wakes and walks the house. One memory remains.", "All four sigils recovered. The chapel doors are waiting."};
    s.lines.push_back({Speaker::Manor, QString::fromUtf8(wrap[std::clamp(sigil, 1, 4)])});
    cine->addShot(std::move(s));
    return cine;
}

} // namespace hm
