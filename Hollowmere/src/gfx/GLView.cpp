#include "GLView.h"
#include "core/Input.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QCursor>

namespace hm {

GLView::GLView(QWidget* parent)
    : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    connect(&m_frameTimer, &QTimer::timeout, this, [this]() {
        update();
    });
    m_frameTimer.start(16); // ~60 FPS update
}

GLView::~GLView() {
    makeCurrent();
    m_renderer.destroy(this);
    doneCurrent();
}

void GLView::setCursorCaptured(bool captured) {
    m_cursorCaptured = captured;
    Input::instance().setMouseLocked(captured);
    if (captured) {
        setCursor(Qt::BlankCursor);
        QPoint center(width() / 2, height() / 2);
        QCursor::setPos(mapToGlobal(center));
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void GLView::initializeGL() {
    initializeOpenGLFunctions();

    const GLubyte* glRenderer = glGetString(GL_RENDERER);
    const GLubyte* glVersion = glGetString(GL_VERSION);
    qInfo("Hollowmere Engine initialized on: %s (OpenGL %s)", glRenderer, glVersion);

    int pixelW = static_cast<int>(width() * devicePixelRatioF());
    int pixelH = static_cast<int>(height() * devicePixelRatioF());

    m_renderer.init(this, pixelW, pixelH);
    m_world.init(this);

    m_clock.reset();
    setCursorCaptured(true);
}

void GLView::resizeGL(int w, int h) {
    int pixelW = static_cast<int>(w * devicePixelRatioF());
    int pixelH = static_cast<int>(h * devicePixelRatioF());
    m_renderer.resize(this, pixelW, pixelH);
}

void GLView::paintGL() {
    // 1. Clock tick & fixed-timestep simulation
    m_clock.tick();
    m_totalTime += static_cast<float>(m_clock.frameTime());

    while (m_clock.hasStep()) {
        m_world.update(static_cast<float>(m_clock.deltaSim()));
        m_clock.consumeStep();
    }

    // Reset single-frame input deltas
    Input::instance().update();

    // 2. 3D Render Pass
    m_renderer.render(this, m_world, m_totalTime);

    // 3. 2D HUD Render Pass using QPainter on top of OpenGL widget
    QPainter painter(this);
    m_hud.draw(painter, rect(), m_world, static_cast<float>(m_clock.fps()));
    painter.end();
}

void GLView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        setCursorCaptured(!m_cursorCaptured);
        return;
    }
    Input::instance().onKeyPressed(event->key());
    event->accept();
}

void GLView::keyReleaseEvent(QKeyEvent* event) {
    Input::instance().onKeyReleased(event->key());
    event->accept();
}

void GLView::mouseMoveEvent(QMouseEvent* event) {
    if (!m_cursorCaptured) return;

    QPoint center(width() / 2, height() / 2);
    int dx = event->pos().x() - center.x();
    int dy = event->pos().y() - center.y();

    if (dx != 0 || dy != 0) {
        Input::instance().onMouseMove(dx, dy);
        QCursor::setPos(mapToGlobal(center));
    }
    event->accept();
}

void GLView::mousePressEvent(QMouseEvent* event) {
    if (!m_cursorCaptured) {
        setCursorCaptured(true);
        return;
    }
    Input::instance().onMouseButtonPressed(event->button());
    event->accept();
}

void GLView::mouseReleaseEvent(QMouseEvent* event) {
    Input::instance().onMouseButtonReleased(event->button());
    event->accept();
}

void GLView::focusOutEvent(QFocusEvent* event) {
    Input::instance().onFocusLost();
    setCursorCaptured(false);
    QOpenGLWidget::focusOutEvent(event);
}

} // namespace hm
