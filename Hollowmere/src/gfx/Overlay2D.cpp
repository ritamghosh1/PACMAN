#include "Overlay2D.h"
#include <QDebug>

namespace hm {

Overlay2D::Overlay2D() = default;

Overlay2D::~Overlay2D() = default;

void Overlay2D::initQuad(QOpenGLFunctions_3_3_Core* gl) {
    if (m_quadVao != 0) return;

    // Fullscreen quad in NDC [-1, 1], with UV flipped vertically to match QImage orientation
    float quadVertices[] = {
        // pos.x, pos.y, uv.x, uv.y
        -1.0f,  1.0f,  0.0f, 0.0f, // top-left
        -1.0f, -1.0f,  0.0f, 1.0f, // bottom-left
         1.0f, -1.0f,  1.0f, 1.0f, // bottom-right

        -1.0f,  1.0f,  0.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f, 0.0f  // top-right
    };

    gl->glGenVertexArrays(1, &m_quadVao);
    gl->glGenBuffers(1, &m_quadVbo);

    gl->glBindVertexArray(m_quadVao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));

    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

    gl->glBindVertexArray(0);
}

bool Overlay2D::init(QOpenGLFunctions_3_3_Core* gl, int pixelWidth, int pixelHeight, qreal dpr) {
    m_pixelW = std::max(1, pixelWidth);
    m_pixelH = std::max(1, pixelHeight);
    m_dpr = (dpr > 0.0) ? dpr : 1.0;

    // Compile 2D blit shader
    const QString vertSrc = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aUV;
        out vec2 vUV;
        void main() {
            vUV = aUV;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    const QString fragSrc = R"(
        #version 330 core
        in vec2 vUV;
        out vec4 FragColor;
        uniform sampler2D uOverlayTex;
        void main() {
            FragColor = texture(uOverlayTex, vUV);
        }
    )";

    if (!m_shader.loadFromSource(gl, vertSrc, fragSrc)) {
        qCritical("Failed to compile Overlay2D shader!");
        return false;
    }

    initQuad(gl);

    // Create texture for QImage upload
    gl->glGenTextures(1, &m_texture);
    gl->glBindTexture(GL_TEXTURE_2D, m_texture);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_pixelW, m_pixelH, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    gl->glBindTexture(GL_TEXTURE_2D, 0);

    // Allocate raster buffer
    m_image = QImage(m_pixelW, m_pixelH, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(Qt::transparent);

    return true;
}

void Overlay2D::resize(QOpenGLFunctions_3_3_Core* gl, int pixelWidth, int pixelHeight, qreal dpr) {
    if (pixelWidth <= 0 || pixelHeight <= 0) return;
    m_pixelW = pixelWidth;
    m_pixelH = pixelHeight;
    m_dpr = (dpr > 0.0) ? dpr : 1.0;

    if (m_texture != 0) {
        gl->glBindTexture(GL_TEXTURE_2D, m_texture);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_pixelW, m_pixelH, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
        gl->glBindTexture(GL_TEXTURE_2D, 0);
    }

    m_image = QImage(m_pixelW, m_pixelH, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(Qt::transparent);
}

void Overlay2D::destroy(QOpenGLFunctions_3_3_Core* gl) {
    if (m_painter) {
        m_painter->end();
        m_painter.reset();
    }
    if (m_texture != 0) {
        gl->glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    if (m_quadVbo != 0) {
        gl->glDeleteBuffers(1, &m_quadVbo);
        m_quadVbo = 0;
    }
    if (m_quadVao != 0) {
        gl->glDeleteVertexArrays(1, &m_quadVao);
        m_quadVao = 0;
    }
}

QPainter* Overlay2D::begin() {
    if (m_image.isNull()) return nullptr;

    m_image.fill(Qt::transparent);
    m_painter = std::make_unique<QPainter>(&m_image);
    m_painter->setRenderHint(QPainter::Antialiasing, true);
    m_painter->setRenderHint(QPainter::TextAntialiasing, true);

    if (m_dpr > 1.0) {
        m_painter->scale(m_dpr, m_dpr);
    }

    return m_painter.get();
}

void Overlay2D::end() {
    if (m_painter) {
        m_painter->end();
        m_painter.reset();
    }
}

void Overlay2D::render(QOpenGLFunctions_3_3_Core* gl, GLuint targetFbo, int pixelW, int pixelH) {
    if (m_image.isNull() || m_texture == 0) return;

    // 1. Upload raster pixel buffer to OpenGL texture
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, m_texture);
    gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_pixelW, m_pixelH, GL_BGRA, GL_UNSIGNED_BYTE, m_image.constBits());

    // 2. Target destination framebuffer & viewport
    gl->glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
    gl->glViewport(0, 0, pixelW, pixelH);

    // 3. Premultiplied alpha blending: source color is pre-multiplied, so GL_ONE + GL_ONE_MINUS_SRC_ALPHA
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    gl->glDisable(GL_DEPTH_TEST);
    gl->glDisable(GL_CULL_FACE);

    // 4. Draw textured quad
    m_shader.bind(gl);
    m_shader.setUniformInt(gl, "uOverlayTex", 0);

    gl->glBindVertexArray(m_quadVao);
    gl->glDrawArrays(GL_TRIANGLES, 0, 6);
    gl->glBindVertexArray(0);

    m_shader.unbind(gl);
    gl->glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace hm
