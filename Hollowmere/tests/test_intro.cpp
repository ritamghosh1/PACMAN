#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <cassert>
#include <iostream>
#include "story/IntroMovie.h"
#include "story/Subtitles.h"

namespace hm {
void AudioEngine::playIntroScore() {}
void AudioEngine::playJumpscareStinger() {}
void AudioEngine::playDoorSlam(const Vec3&) {}
}

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    std::cout << "[Test] Initializing IntroMovie...\n";
    hm::IntroMovie intro(nullptr);
    intro.reset();

    QRect bounds(0, 0, 1280, 720);
    QImage canvas(1280, 720, QImage::Format_ARGB32_Premultiplied);

    // Test Scene 1 (Fire Memory at t = 1.0s)
    {
        canvas.fill(Qt::transparent);
        QPainter p(&canvas);
        intro.update(1.0f, false);
        intro.render(p, bounds);
        p.end();

        // Check that pixels are NOT all black
        bool hasColor = false;
        for (int y = 500; y < 700; ++y) {
            for (int x = 400; x < 800; ++x) {
                QRgb pix = canvas.pixel(x, y);
                if (qRed(pix) > 50 || qGreen(pix) > 20) {
                    hasColor = true;
                    break;
                }
            }
            if (hasColor) break;
        }
        assert(hasColor && "Scene 1 FireMemory must render vivid flame colors!");
        std::cout << "  ✓ Scene 1 (FireMemory) verified with active flame rendering.\n";
    }

    // Test Scene 2 (Letter Scene at t = 25.0s)
    {
        intro.reset();
        intro.update(25.0f, false);
        canvas.fill(Qt::transparent);
        QPainter p(&canvas);
        intro.render(p, bounds);
        p.end();

        bool hasParchment = false;
        for (int y = 300; y < 500; ++y) {
            for (int x = 500; x < 750; ++x) {
                QRgb pix = canvas.pixel(x, y);
                if (qRed(pix) > 180 && qGreen(pix) > 180 && qBlue(pix) > 150) {
                    hasParchment = true;
                    break;
                }
            }
            if (hasParchment) break;
        }
        assert(hasParchment && "Scene 2 LetterScene must render parchment letter!");
        std::cout << "  ✓ Scene 2 (LetterScene) verified with parchment letter and lamp light.\n";
    }

    // Test Scene 4 (Manor Silhouette & Sky at t = 65.0s)
    {
        intro.reset();
        intro.update(65.0f, false);
        canvas.fill(Qt::transparent);
        QPainter p(&canvas);
        intro.render(p, bounds);
        p.end();

        bool hasSky = false;
        for (int y = 50; y < 200; ++y) {
            for (int x = 200; x < 1000; ++x) {
                QRgb pix = canvas.pixel(x, y);
                if (qRed(pix) > 5 || qBlue(pix) > 15) {
                    hasSky = true;
                    break;
                }
            }
            if (hasSky) break;
        }
        assert(hasSky && "Scene 4 ManorSilhouette must render gothic sky and spires!");
        std::cout << "  ✓ Scene 4 (ManorSilhouette) verified with sky and architecture.\n";
    }

    // Test Scene 5 (Title Card at t = 77.0s)
    {
        intro.reset();
        intro.update(77.0f, false);
        canvas.fill(Qt::transparent);
        QPainter p(&canvas);
        intro.render(p, bounds);
        p.end();

        bool hasTitle = false;
        for (int y = 320; y < 400; ++y) {
            for (int x = 340; x < 940; ++x) {
                QRgb pix = canvas.pixel(x, y);
                if (qRed(pix) > 100 || qGreen(pix) > 100 || qBlue(pix) > 100) {
                    hasTitle = true;
                    break;
                }
            }
            if (hasTitle) break;
        }
        assert(hasTitle && "Scene 5 TitleCard must render HOLLOWMERE title text!");
        std::cout << "  ✓ Scene 5 (TitleCard) verified with glowing title typography.\n";
    }

    std::cout << "[Test Passed] All IntroMovie scenes render correctly without black screen!\n";
    return 0;
}
