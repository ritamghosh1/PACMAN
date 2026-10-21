#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QImage>

namespace hm {

class Texture {
public:
    Texture();
    ~Texture();

    bool createFromImage(QOpenGLFunctions_3_3_Core* gl, const QImage& image, bool generateMipmaps = true);
    bool createFromPixels(QOpenGLFunctions_3_3_Core* gl, const uint8_t* rgba, int width, int height, bool generateMipmaps = true);

    void bind(QOpenGLFunctions_3_3_Core* gl, GLuint slot = 0) const;
    void unbind(QOpenGLFunctions_3_3_Core* gl, GLuint slot = 0) const;

    void destroy(QOpenGLFunctions_3_3_Core* gl);

    GLuint id() const { return m_textureId; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    bool isValid() const { return m_textureId != 0; }

private:
    GLuint m_textureId = 0;
    int m_width = 0;
    int m_height = 0;
};

} // namespace hm
