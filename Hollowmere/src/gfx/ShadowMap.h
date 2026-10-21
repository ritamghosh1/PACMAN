#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include "core/mathx.h"
#include "Shader.h"

namespace hm {

class ShadowMap {
public:
    ShadowMap();
    ~ShadowMap();

    bool init(QOpenGLFunctions_3_3_Core* gl, int resolution = 1024);
    void begin(QOpenGLFunctions_3_3_Core* gl, const Vec3& lightPos, const Vec3& lightDir, float outerCutoffDeg, float range);
    void end(QOpenGLFunctions_3_3_Core* gl);
    void bindDepthTexture(QOpenGLFunctions_3_3_Core* gl, GLuint slot = 2) const;
    void destroy(QOpenGLFunctions_3_3_Core* gl);

    const Mat4& lightSpaceMatrix() const { return m_lightSpaceMatrix; }
    GLuint depthTexture() const { return m_depthTex; }
    int resolution() const { return m_resolution; }
    Shader& depthShader() { return m_depthShader; }

private:
    GLuint m_fbo = 0;
    GLuint m_depthTex = 0;
    int m_resolution = 1024;
    Mat4 m_lightSpaceMatrix;
    Shader m_depthShader;
};

} // namespace hm
