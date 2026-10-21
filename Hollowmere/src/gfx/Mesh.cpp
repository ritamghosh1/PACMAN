#include "Mesh.h"

namespace hm {

Mesh::Mesh() = default;

Mesh::~Mesh() = default;

void Mesh::upload(QOpenGLFunctions_3_3_Core* gl,
                  const std::vector<Vertex>& vertices,
                  const std::vector<uint32_t>& indices) {
    destroy(gl);

    m_vertexCount = vertices.size();
    m_indexCount = indices.size();
    if (m_vertexCount == 0 || m_indexCount == 0) return;

    gl->glGenVertexArrays(1, &m_vao);
    gl->glGenBuffers(1, &m_vbo);
    gl->glGenBuffers(1, &m_ebo);

    gl->glBindVertexArray(m_vao);

    gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    gl->glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                     vertices.data(),
                     GL_STATIC_DRAW);

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
                     indices.data(),
                     GL_STATIC_DRAW);

    GLsizei stride = sizeof(Vertex);

    // 0: Position (Vec3)
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, pos)));

    // 1: Normal (Vec3)
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, normal)));

    // 2: UV (Vec2)
    gl->glEnableVertexAttribArray(2);
    gl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, uv)));

    // 3: Tangent (Vec3)
    gl->glEnableVertexAttribArray(3);
    gl->glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, tangent)));

    // 4: Color (Vec4)
    gl->glEnableVertexAttribArray(4);
    gl->glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, color)));

    gl->glBindVertexArray(0);
}

void Mesh::draw(QOpenGLFunctions_3_3_Core* gl) const {
    if (m_vao != 0 && m_indexCount > 0) {
        gl->glBindVertexArray(m_vao);
        gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, nullptr);
        gl->glBindVertexArray(0);
    }
}

void Mesh::destroy(QOpenGLFunctions_3_3_Core* gl) {
    if (m_ebo != 0) {
        gl->glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
    if (m_vbo != 0) {
        gl->glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        gl->glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    m_indexCount = 0;
    m_vertexCount = 0;
}

} // namespace hm
