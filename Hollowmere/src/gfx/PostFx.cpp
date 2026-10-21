#include "PostFx.h"
#include <QFile>

namespace hm {

PostFx::PostFx() = default;

PostFx::~PostFx() = default;

void PostFx::initQuad(QOpenGLFunctions_3_3_Core* gl) {
    if (m_quadVao != 0) return;

    // Fullscreen quad in NDC [-1, 1]
    float quadVertices[] = {
        // pos.x, pos.y, uv.x, uv.y
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
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

bool PostFx::init(QOpenGLFunctions_3_3_Core* gl, int width, int height) {
    m_shader.loadFromFiles(gl, ":/shaders/post.vert", ":/shaders/post.frag");
    initQuad(gl);
    resize(gl, width, height);
    return true;
}

void PostFx::resize(QOpenGLFunctions_3_3_Core* gl, int width, int height) {
    if (width <= 0 || height <= 0) return;
    m_width = width;
    m_height = height;

    destroy(gl);

    gl->glGenFramebuffers(1, &m_fbo);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Color texture
    gl->glGenTextures(1, &m_colorTex);
    gl->glBindTexture(GL_TEXTURE_2D, m_colorTex);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    // Depth & Stencil renderbuffer
    gl->glGenRenderbuffers(1, &m_rboDepth);
    gl->glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
    gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);

    if (gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        qCritical("PostFx Framebuffer is incomplete!");
    }

    gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostFx::begin(QOpenGLFunctions_3_3_Core* gl) {
    gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    gl->glViewport(0, 0, m_width, m_height);
    gl->glClearColor(0.015f, 0.015f, 0.02f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostFx::end(QOpenGLFunctions_3_3_Core* gl) {
    gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostFx::render(QOpenGLFunctions_3_3_Core* gl, float time, float fear, float health, float sanity) {
    gl->glDisable(GL_DEPTH_TEST);
    m_shader.bind(gl);

    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, m_colorTex);
    m_shader.setUniformInt(gl, "uSceneTex", 0);

    m_shader.setUniformFloat(gl, "uTime", time);
    m_shader.setUniformFloat(gl, "uFear", fear);
    m_shader.setUniformFloat(gl, "uHealth", health);
    m_shader.setUniformFloat(gl, "uSanity", sanity);

    gl->glBindVertexArray(m_quadVao);
    gl->glDrawArrays(GL_TRIANGLES, 0, 6);
    gl->glBindVertexArray(0);

    m_shader.unbind(gl);
    gl->glEnable(GL_DEPTH_TEST);
}

void PostFx::destroy(QOpenGLFunctions_3_3_Core* gl) {
    if (m_colorTex != 0) {
        gl->glDeleteTextures(1, &m_colorTex);
        m_colorTex = 0;
    }
    if (m_rboDepth != 0) {
        gl->glDeleteRenderbuffers(1, &m_rboDepth);
        m_rboDepth = 0;
    }
    if (m_fbo != 0) {
        gl->glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}

} // namespace hm
