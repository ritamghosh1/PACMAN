#include "Texture.h"

namespace hm {

Texture::Texture() = default;

Texture::~Texture() = default;

bool Texture::createFromImage(QOpenGLFunctions_3_3_Core* gl, const QImage& image, bool generateMipmaps) {
    if (image.isNull()) return false;

    QImage glImg = image.convertToFormat(QImage::Format_RGBA8888);
    return createFromPixels(gl, glImg.constBits(), glImg.width(), glImg.height(), generateMipmaps);
}

bool Texture::createFromPixels(QOpenGLFunctions_3_3_Core* gl, const uint8_t* rgba, int width, int height, bool generateMipmaps) {
    destroy(gl);

    m_width = width;
    m_height = height;

    gl->glGenTextures(1, &m_textureId);
    gl->glBindTexture(GL_TEXTURE_2D, m_textureId);

    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (generateMipmaps) {
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

    if (generateMipmaps) {
        gl->glGenerateMipmap(GL_TEXTURE_2D);
    }

    gl->glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Texture::bind(QOpenGLFunctions_3_3_Core* gl, GLuint slot) const {
    gl->glActiveTexture(GL_TEXTURE0 + slot);
    gl->glBindTexture(GL_TEXTURE_2D, m_textureId);
}

void Texture::unbind(QOpenGLFunctions_3_3_Core* gl, GLuint slot) const {
    gl->glActiveTexture(GL_TEXTURE0 + slot);
    gl->glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::destroy(QOpenGLFunctions_3_3_Core* gl) {
    if (m_textureId != 0) {
        gl->glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
    m_width = 0;
    m_height = 0;
}

} // namespace hm
