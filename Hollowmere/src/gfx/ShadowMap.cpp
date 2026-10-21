#include "ShadowMap.h"
#include <QDebug>

namespace hm {

ShadowMap::ShadowMap() = default;

ShadowMap::~ShadowMap() = default;

bool ShadowMap::init(QOpenGLFunctions_3_3_Core* gl, int resolution) {
    m_resolution = resolution;
    destroy(gl);

    if (!m_depthShader.loadFromFiles(gl, ":/shaders/shadow.vert", ":/shaders/shadow.frag")) {
        qWarning("Failed to load shadow shaders!");
        return false;
    }

    gl->glGenFramebuffers(1, &m_fbo);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    gl->glGenTextures(1, &m_depthTex);
    gl->glBindTexture(GL_TEXTURE_2D, m_depthTex);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_resolution, m_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    gl->glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);
    gl->glDrawBuffer(GL_NONE);
    gl->glReadBuffer(GL_NONE);

    if (gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        qCritical("ShadowMap framebuffer is incomplete!");
        gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void ShadowMap::begin(QOpenGLFunctions_3_3_Core* gl, const Vec3& lightPos, const Vec3& lightDir, float outerCutoffDeg, float range) {
    gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    gl->glViewport(0, 0, m_resolution, m_resolution);
    gl->glClear(GL_DEPTH_BUFFER_BIT);

    Vec3 target = lightPos + normalize(lightDir);
    Vec3 up(0, 1, 0);
    if (std::abs(dot(normalize(lightDir), up)) > 0.95f) {
        up = Vec3(0, 0, 1);
    }

    Mat4 lightView = Mat4::lookAt(lightPos, target, up);
    float fovRad = radians(std::min(120.0f, outerCutoffDeg * 2.0f + 10.0f));
    Mat4 lightProj = Mat4::perspective(fovRad, 1.0f, 0.1f, range);
    m_lightSpaceMatrix = lightProj * lightView;

    m_depthShader.bind(gl);
    m_depthShader.setUniformMat4(gl, "uLightSpaceMatrix", m_lightSpaceMatrix);
}

void ShadowMap::end(QOpenGLFunctions_3_3_Core* gl) {
    m_depthShader.unbind(gl);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::bindDepthTexture(QOpenGLFunctions_3_3_Core* gl, GLuint slot) const {
    gl->glActiveTexture(GL_TEXTURE0 + slot);
    gl->glBindTexture(GL_TEXTURE_2D, m_depthTex);
}

void ShadowMap::destroy(QOpenGLFunctions_3_3_Core* gl) {
    if (m_depthTex != 0) {
        gl->glDeleteTextures(1, &m_depthTex);
        m_depthTex = 0;
    }
    if (m_fbo != 0) {
        gl->glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}

} // namespace hm
