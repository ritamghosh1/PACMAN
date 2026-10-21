#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QTimer>
#include "Renderer.h"
#include "world/World.h"
#include "core/Clock.h"
#include "ui/Hud.h"

namespace hm {

class GLView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit GLView(QWidget* parent = nullptr);
    ~GLView() override;

    void setCursorCaptured(bool captured);
    bool isCursorCaptured() const { return m_cursorCaptured; }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    Renderer m_renderer;
    World m_world;
    Clock m_clock;
    Hud m_hud;

    QTimer m_frameTimer;
    float m_totalTime = 0.0f;
    bool m_cursorCaptured = true;
};

} // namespace hm
