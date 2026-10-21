#pragma once

#include <vector>
#include "Mesh.h"

namespace hm {

class MeshBuilder {
public:
    MeshBuilder() = default;

    void clear();

    void addQuad(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3,
                 const Vec2& uv0, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3,
                 const Vec3& normal, const Vec4& color = Vec4(1, 1, 1, 1));

    void addBox(const Vec3& center, const Vec3& size, const Vec2& uvScale = Vec2(1, 1),
                const Vec4& color = Vec4(1, 1, 1, 1));

    void addCylinder(const Vec3& base, float radius, float height, int segments = 12,
                     const Vec4& color = Vec4(1, 1, 1, 1));

    void addSphere(const Vec3& center, float radius, int rings = 10, int sectors = 12,
                   const Vec4& color = Vec4(1, 1, 1, 1));

    void build(QOpenGLFunctions_3_3_Core* gl, Mesh& outMesh) const;

    const std::vector<Vertex>& vertices() const { return m_vertices; }
    const std::vector<uint32_t>& indices() const { return m_indices; }

private:
    void computeTangent(const Vertex& v0, const Vertex& v1, const Vertex& v2, Vec3& tangent);

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};

} // namespace hm
