#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QImage>
#include <QPainter>
#include <memory>
#include "Shader.h"

namespace hm {

class Overlay2D {
public:
    Overlay2D();
    ~Overlay2D();

    bool init(QOpenGLFunctions_3_3_Core* gl, int pixelWidth, int pixelHeight, qreal dpr);
    void resize(QOpenGLFunctions_3_3_Core* gl, int pixelWidth, int pixelHeight, qreal dpr);
    void destroy(QOpenGLFunctions_3_3_Core* gl);

    // Begins painting on the internal raster canvas; returns active QPainter
    QPainter* begin();
    // Finishes painting on the raster canvas
    void end();

    // Renders the overlay texture to the target framebuffer using OpenGL 3.3 Core Profile
    void render(QOpenGLFunctions_3_3_Core* gl, GLuint targetFbo, int pixelW, int pixelH);

    int pixelWidth() const { return m_pixelW; }
    int pixelHeight() const { return m_pixelH; }
    qreal dpr() const { return m_dpr; }

private:
    void initQuad(QOpenGLFunctions_3_3_Core* gl);

    int m_pixelW = 1280;
    int m_pixelH = 720;
    qreal m_dpr = 1.0;

    QImage m_image;
    std::unique_ptr<QPainter> m_painter;

    GLuint m_texture = 0;
    GLuint m_quadVao = 0;
    GLuint m_quadVbo = 0;
    Shader m_shader;
};

} // namespace hm
