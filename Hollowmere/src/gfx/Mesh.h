#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <vector>
#include "core/mathx.h"

namespace hm {

struct Vertex {
    Vec3 pos;
    Vec3 normal;
    Vec2 uv;
    Vec3 tangent;
    Vec4 color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

class Mesh {
public:
    Mesh();
    ~Mesh();

    void upload(QOpenGLFunctions_3_3_Core* gl,
                const std::vector<Vertex>& vertices,
                const std::vector<uint32_t>& indices);

    void draw(QOpenGLFunctions_3_3_Core* gl) const;
    void destroy(QOpenGLFunctions_3_3_Core* gl);

    size_t indexCount() const { return m_indexCount; }
    size_t vertexCount() const { return m_vertexCount; }
    bool isValid() const { return m_vao != 0; }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    size_t m_indexCount = 0;
    size_t m_vertexCount = 0;
};

} // namespace hm
