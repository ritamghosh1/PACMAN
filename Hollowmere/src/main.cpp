#include <QApplication>
#include <QSurfaceFormat>
#include "app/MainWindow.h"

int main(int argc, char* argv[]) {
    // 1. OpenGL 3.3 Core profile format MUST be set before QApplication
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(0); // the scene is drawn into an off-screen FBO, so window MSAA only costs fill rate
    format.setSwapInterval(1); // VSync
    QSurfaceFormat::setDefaultFormat(format);

    // 2. High DPI scaling attributes
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setApplicationName("Hollowmere");
    app.setOrganizationName("AntigravityHorror");

    hm::MainWindow window;
    window.show();

    return app.exec();
}
