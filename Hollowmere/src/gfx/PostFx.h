#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include "Shader.h"

namespace hm {

class PostFx {
public:
    PostFx();
    ~PostFx();

    bool init(QOpenGLFunctions_3_3_Core* gl, int width, int height);
    void resize(QOpenGLFunctions_3_3_Core* gl, int width, int height);

    void begin(QOpenGLFunctions_3_3_Core* gl);
    void end(QOpenGLFunctions_3_3_Core* gl);

    void render(QOpenGLFunctions_3_3_Core* gl, float time, float fear, float health, float sanity);

    void destroy(QOpenGLFunctions_3_3_Core* gl);

    GLuint fbo() const { return m_fbo; }
    GLuint sceneTexture() const { return m_colorTex; }

private:
    void initQuad(QOpenGLFunctions_3_3_Core* gl);

    GLuint m_fbo = 0;
    GLuint m_colorTex = 0;
    GLuint m_rboDepth = 0;

    GLuint m_quadVao = 0;
    GLuint m_quadVbo = 0;

    int m_width = 0;
    int m_height = 0;

    Shader m_shader;
};

} // namespace hm
