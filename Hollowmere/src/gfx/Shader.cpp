#include "Shader.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <vector>

namespace hm {

Shader::Shader() = default;

Shader::~Shader() {
    // Note: destruction requires active context, handled explicitly if needed
}

bool Shader::loadFromFiles(QOpenGLFunctions_3_3_Core* gl, const QString& vertPath, const QString& fragPath) {
    QFile vertFile(vertPath);
    if (!vertFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Failed to open vertex shader file: %s", qPrintable(vertPath));
        return false;
    }
    QString vertSrc = vertFile.readAll();

    QFile fragFile(fragPath);
    if (!fragFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Failed to open fragment shader file: %s", qPrintable(fragPath));
        return false;
    }
    QString fragSrc = fragFile.readAll();

    return loadFromSource(gl, vertSrc, fragSrc);
}

GLuint Shader::compileShader(QOpenGLFunctions_3_3_Core* gl, GLenum type, const std::string& src) {
    GLuint shader = gl->glCreateShader(type);
    const char* cstr = src.c_str();
    gl->glShaderSource(shader, 1, &cstr, nullptr);
    gl->glCompileShader(shader);

    GLint success = 0;
    gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> infoLog(logLen + 1, 0);
        gl->glGetShaderInfoLog(shader, logLen, nullptr, infoLog.data());
        qCritical("Shader compilation failed (%s):\n%s",
                  (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT"),
                  infoLog.data());
        gl->glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::loadFromSource(QOpenGLFunctions_3_3_Core* gl, const QString& vertSrc, const QString& fragSrc) {
    if (m_program != 0) {
        gl->glDeleteProgram(m_program);
        m_program = 0;
    }
    m_uniformLocs.clear();

    GLuint vert = compileShader(gl, GL_VERTEX_SHADER, vertSrc.toStdString());
    if (!vert) return false;

    GLuint frag = compileShader(gl, GL_FRAGMENT_SHADER, fragSrc.toStdString());
    if (!frag) {
        gl->glDeleteShader(vert);
        return false;
    }

    m_program = gl->glCreateProgram();
    gl->glAttachShader(m_program, vert);
    gl->glAttachShader(m_program, frag);
    gl->glLinkProgram(m_program);

    GLint success = 0;
    gl->glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        gl->glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> infoLog(logLen + 1, 0);
        gl->glGetProgramInfoLog(m_program, logLen, nullptr, infoLog.data());
        qCritical("Shader program link failed:\n%s", infoLog.data());
        gl->glDeleteProgram(m_program);
        m_program = 0;
        gl->glDeleteShader(vert);
        gl->glDeleteShader(frag);
        return false;
    }

    gl->glDeleteShader(vert);
    gl->glDeleteShader(frag);
    return true;
}

void Shader::bind(QOpenGLFunctions_3_3_Core* gl) {
    if (m_program != 0) {
        gl->glUseProgram(m_program);
    }
}

void Shader::unbind(QOpenGLFunctions_3_3_Core* gl) {
    gl->glUseProgram(0);
}

GLint Shader::getUniformLoc(QOpenGLFunctions_3_3_Core* gl, const char* name) {
    auto it = m_uniformLocs.find(name);
    if (it != m_uniformLocs.end()) {
        return it->second;
    }
    GLint loc = gl->glGetUniformLocation(m_program, name);
    m_uniformLocs[name] = loc;
    return loc;
}

void Shader::setUniformInt(QOpenGLFunctions_3_3_Core* gl, const char* name, int val) {
    GLint loc = getUniformLoc(gl, name);
    if (loc >= 0) gl->glUniform1i(loc, val);
}

void Shader::setUniformFloat(QOpenGLFunctions_3_3_Core* gl, const char* name, float val) {
    GLint loc = getUniformLoc(gl, name);
    if (loc >= 0) gl->glUniform1f(loc, val);
}

void Shader::setUniformVec2(QOpenGLFunctions_3_3_Core* gl, const char* name, const Vec2& val) {
    GLint loc = getUniformLoc(gl, name);
    if (loc >= 0) gl->glUniform2f(loc, val.x, val.y);
}

void Shader::setUniformVec3(QOpenGLFunctions_3_3_Core* gl, const char* name, const Vec3& val) {
    GLint loc = getUniformLoc(gl, name);
    if (loc >= 0) gl->glUniform3f(loc, val.x, val.y, val.z);
}

void Shader::setUniformVec4(QOpenGLFunctions_3_3_Core* gl, const char* name, const Vec4& val) {
    GLint loc = getUniformLoc(gl, name);
    if (loc >= 0) gl->glUniform4f(loc, val.x, val.y, val.z, val.w);
}

void Shader::setUniformMat4(QOpenGLFunctions_3_3_Core* gl, const char* name, const Mat4& val) {
    GLint loc = getUniformLoc(gl, name);
    if (loc >= 0) gl->glUniformMatrix4fv(loc, 1, GL_FALSE, val.data());
}

} // namespace hm
