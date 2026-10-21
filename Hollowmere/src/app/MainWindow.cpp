#include "MainWindow.h"
#include <QKeyEvent>

namespace hm {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Hollowmere - Psychological Horror");
    resize(1280, 720);

    m_glView = new GLView(this);
    setCentralWidget(m_glView);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_F11) {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
        event->accept();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

} // namespace hm
