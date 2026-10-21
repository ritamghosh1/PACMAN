#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QString>
#include <unordered_map>
#include <string>
#include "core/mathx.h"

namespace hm {

class Shader {
public:
    Shader();
    ~Shader();

    bool loadFromSource(QOpenGLFunctions_3_3_Core* gl, const QString& vertSrc, const QString& fragSrc);
    bool loadFromFiles(QOpenGLFunctions_3_3_Core* gl, const QString& vertPath, const QString& fragPath);

    void bind(QOpenGLFunctions_3_3_Core* gl);
    void unbind(QOpenGLFunctions_3_3_Core* gl);

    GLuint programId() const { return m_program; }
    bool isValid() const { return m_program != 0; }

    void setUniformInt(QOpenGLFunctions_3_3_Core* gl, const char* name, int val);
    void setUniformFloat(QOpenGLFunctions_3_3_Core* gl, const char* name, float val);
    void setUniformVec2(QOpenGLFunctions_3_3_Core* gl, const char* name, const Vec2& val);
    void setUniformVec3(QOpenGLFunctions_3_3_Core* gl, const char* name, const Vec3& val);
    void setUniformVec4(QOpenGLFunctions_3_3_Core* gl, const char* name, const Vec4& val);
    void setUniformMat4(QOpenGLFunctions_3_3_Core* gl, const char* name, const Mat4& val);

private:
    GLint getUniformLoc(QOpenGLFunctions_3_3_Core* gl, const char* name);
    GLuint compileShader(QOpenGLFunctions_3_3_Core* gl, GLenum type, const std::string& src);

    GLuint m_program = 0;
    std::unordered_map<std::string, GLint> m_uniformLocs;
};

} // namespace hm
