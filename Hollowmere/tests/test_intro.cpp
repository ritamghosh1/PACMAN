// Headless checks for the typewriter dialogue and the intro / ending cinematics.
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <cassert>
#include <iostream>
#include "story/Cinematic.h"
#include "story/Cutscenes.h"
#include "story/Dialogue.h"

using namespace hm;

int main(int argc, char* argv[]) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);

    // Typewriter: text appears gradually, first advance completes it, second moves on.
    {
        DialogueBox box;
        box.push(Speaker::Nora, "Mae? Mae, are you there?");
        box.push(Speaker::Narrator, "Silence.");
        assert(box.active() && box.typing());
        box.update(0.1f);
        assert(box.typing());                 // still typing after a fraction of a second
        assert(box.advance() && box.lineComplete());
        assert(box.advance() && box.active()); // moved to the second line
        assert(box.advance());
        assert(box.advance() == false || !box.active());
        std::cout << "  ok typewriter + click-to-advance\n";
    }

    QImage canvas(1280, 720, QImage::Format_ARGB32_Premultiplied);
    QRect bounds(0, 0, 1280, 720);
    auto runThrough = [&](Cinematic& c, const char* name) {
        c.start();
        float t = 0, idle = 0;
        while (!c.finished() && t < 600.0f) {
            c.update(1.0f / 30.0f);
            t += 1.0f / 30.0f;
            idle += 1.0f / 30.0f;
            if (idle > 2.0f) { c.onAdvanceKey(); idle = 1.2f; }
            if (static_cast<int>(t * 30) % 30 == 0) {
                canvas.fill(Qt::black);
                QPainter p(&canvas);
                c.render(p, bounds);
            }
        }
        assert(c.finished());
        std::cout << "  ok " << name << " plays to the end in " << t << "s\n";
    };

    auto intro = makeIntroCinematic(nullptr);
    assert(intro->shotCount() >= 6);
    runThrough(*intro, "intro");
    for (int e = 0; e < 5; ++e) {
        auto end = makeEndingCinematic(static_cast<EndingType>(e), nullptr);
        runThrough(*end, "ending");
    }
    auto skipped = makeIntroCinematic(nullptr);
    skipped->start();
    skipped->skip();
    assert(skipped->finished());
    std::cout << "[TEST] ALL CINEMATIC TESTS PASSED\n";
    return 0;
}
