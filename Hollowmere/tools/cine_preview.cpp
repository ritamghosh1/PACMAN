// Offline cutscene preview: renders a cinematic to PNG frames without OpenGL or audio.
//   cine_preview <intro|ending0..4> <outDir> [everySeconds=3] [maxSeconds=300]
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QDir>
#include <cstdio>
#include <cstdlib>
#include "story/Cutscenes.h"

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    if (argc < 3) { std::fprintf(stderr, "usage: cine_preview <intro|endingN> <outDir> [every] [max]\n"); return 1; }
    QString which = argv[1];
    QString out = argv[2];
    float every = argc > 3 ? std::atof(argv[3]) : 3.0f;
    float maxT = argc > 4 ? std::atof(argv[4]) : 300.0f;
    QDir().mkpath(out);

    std::unique_ptr<hm::Cinematic> cine;
    if (which == "intro") cine = hm::makeIntroCinematic(nullptr);
    else cine = hm::makeEndingCinematic(static_cast<hm::EndingType>(which.mid(6).toInt()), nullptr);

    QImage img(1280, 720, QImage::Format_ARGB32_Premultiplied);
    QRect bounds(0, 0, 1280, 720);
    const float dt = 1.0f / 30.0f;
    float t = 0, nextShot = 0, idle = 0;
    int n = 0;
    cine->start();
    while (!cine->finished() && t < maxT) {
        cine->update(dt);
        t += dt;
        idle += dt;
        // Pretend to be a reader: tap through each line a moment after it has finished typing.
        if (idle > 9.0f) { cine->onAdvanceKey(); idle = 1.4f; }
        if (t >= nextShot) {
            img.fill(Qt::black);
            QPainter p(&img);
            cine->render(p, bounds);
            p.end();
            img.save(QString("%1/f%2_s%3.png").arg(out).arg(n++, 3, 10, QChar('0')).arg(cine->shotIndex()));
            nextShot += every;
        }
    }
    std::printf("frames=%d simSeconds=%.1f finished=%d\n", n, t, cine->finished());
    return 0;
}
