#include "MeshBuilder.h"

namespace hm {

void MeshBuilder::clear() {
    m_vertices.clear();
    m_indices.clear();
}

void MeshBuilder::computeTangent(const Vertex& v0, const Vertex& v1, const Vertex& v2, Vec3& tangent) {
    Vec3 edge1 = v1.pos - v0.pos;
    Vec3 edge2 = v2.pos - v0.pos;
    Vec2 deltaUV1 = v1.uv - v0.uv;
    Vec2 deltaUV2 = v2.uv - v0.uv;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y + 1e-6f);
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent = normalize(tangent);
}

void MeshBuilder::addQuad(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3,
                          const Vec2& uv0, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3,
                          const Vec3& normal, const Vec4& color) {
    uint32_t baseIdx = static_cast<uint32_t>(m_vertices.size());

    Vertex v0{p0, normal, uv0, Vec3(1, 0, 0), color};
    Vertex v1{p1, normal, uv1, Vec3(1, 0, 0), color};
    Vertex v2{p2, normal, uv2, Vec3(1, 0, 0), color};
    Vertex v3{p3, normal, uv3, Vec3(1, 0, 0), color};

    Vec3 tangent;
    computeTangent(v0, v1, v2, tangent);
    v0.tangent = tangent;
    v1.tangent = tangent;
    v2.tangent = tangent;
    v3.tangent = tangent;

    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
    m_vertices.push_back(v3);

    // Two triangles: (0, 1, 2) and (0, 2, 3)
    m_indices.push_back(baseIdx + 0);
    m_indices.push_back(baseIdx + 1);
    m_indices.push_back(baseIdx + 2);

    m_indices.push_back(baseIdx + 0);
    m_indices.push_back(baseIdx + 2);
    m_indices.push_back(baseIdx + 3);
}

void MeshBuilder::addBox(const Vec3& center, const Vec3& size, const Vec2& uvScale, const Vec4& color) {
    Vec3 h = size * 0.5f;

    // Front (+Z)
    addQuad(
        center + Vec3(-h.x, -h.y,  h.z),
        center + Vec3( h.x, -h.y,  h.z),
        center + Vec3( h.x,  h.y,  h.z),
        center + Vec3(-h.x,  h.y,  h.z),
        Vec2(0, 0), Vec2(uvScale.x, 0), Vec2(uvScale.x, uvScale.y), Vec2(0, uvScale.y),
        Vec3(0, 0, 1), color
    );

    // Back (-Z)
    addQuad(
        center + Vec3( h.x, -h.y, -h.z),
        center + Vec3(-h.x, -h.y, -h.z),
        center + Vec3(-h.x,  h.y, -h.z),
        center + Vec3( h.x,  h.y, -h.z),
        Vec2(0, 0), Vec2(uvScale.x, 0), Vec2(uvScale.x, uvScale.y), Vec2(0, uvScale.y),
        Vec3(0, 0, -1), color
    );

    // Top (+Y)
    addQuad(
        center + Vec3(-h.x,  h.y,  h.z),
        center + Vec3( h.x,  h.y,  h.z),
        center + Vec3( h.x,  h.y, -h.z),
        center + Vec3(-h.x,  h.y, -h.z),
        Vec2(0, 0), Vec2(uvScale.x, 0), Vec2(uvScale.x, uvScale.y), Vec2(0, uvScale.y),
        Vec3(0, 1, 0), color
    );

    // Bottom (-Y)
    addQuad(
        center + Vec3(-h.x, -h.y, -h.z),
        center + Vec3( h.x, -h.y, -h.z),
        center + Vec3( h.x, -h.y,  h.z),
        center + Vec3(-h.x, -h.y,  h.z),
        Vec2(0, 0), Vec2(uvScale.x, 0), Vec2(uvScale.x, uvScale.y), Vec2(0, uvScale.y),
        Vec3(0, -1, 0), color
    );

    // Right (+X)
    addQuad(
        center + Vec3( h.x, -h.y,  h.z),
        center + Vec3( h.x, -h.y, -h.z),
        center + Vec3( h.x,  h.y, -h.z),
        center + Vec3( h.x,  h.y,  h.z),
        Vec2(0, 0), Vec2(uvScale.x, 0), Vec2(uvScale.x, uvScale.y), Vec2(0, uvScale.y),
        Vec3(1, 0, 0), color
    );

    // Left (-X)
    addQuad(
        center + Vec3(-h.x, -h.y, -h.z),
        center + Vec3(-h.x, -h.y,  h.z),
        center + Vec3(-h.x,  h.y,  h.z),
        center + Vec3(-h.x,  h.y, -h.z),
        Vec2(0, 0), Vec2(uvScale.x, 0), Vec2(uvScale.x, uvScale.y), Vec2(0, uvScale.y),
        Vec3(-1, 0, 0), color
    );
}

void MeshBuilder::addCylinder(const Vec3& base, float radius, float height, int segments, const Vec4& color) {
    if (segments < 3) segments = 3;
    float step = TWO_PI / segments;

    for (int i = 0; i < segments; ++i) {
        float a0 = i * step;
        float a1 = (i + 1) * step;

        float c0 = std::cos(a0);
        float s0 = std::sin(a0);
        float c1 = std::cos(a1);
        float s1 = std::sin(a1);

        Vec3 p0 = base + Vec3(c0 * radius, 0.0f, s0 * radius);
        Vec3 p1 = base + Vec3(c1 * radius, 0.0f, s1 * radius);
        Vec3 p2 = p1 + Vec3(0.0f, height, 0.0f);
        Vec3 p3 = p0 + Vec3(0.0f, height, 0.0f);

        Vec3 n0 = normalize(Vec3(c0, 0.0f, s0));
        Vec3 n1 = normalize(Vec3(c1, 0.0f, s1));
        Vec3 avgN = normalize(n0 + n1);

        float u0 = static_cast<float>(i) / segments;
        float u1 = static_cast<float>(i + 1) / segments;

        addQuad(p0, p1, p2, p3,
                Vec2(u0, 0.0f), Vec2(u1, 0.0f), Vec2(u1, 1.0f), Vec2(u0, 1.0f),
                avgN, color);
    }
}

void MeshBuilder::addSphere(const Vec3& center, float radius, int rings, int sectors, const Vec4& color) {
    float const R = 1.0f / (float)(rings - 1);
    float const S = 1.0f / (float)(sectors - 1);

    uint32_t baseIdx = static_cast<uint32_t>(m_vertices.size());

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            float y = std::sin(-PI * 0.5f + PI * r * R);
            float x = std::cos(2.0f * PI * s * S) * std::sin(PI * r * R);
            float z = std::sin(2.0f * PI * s * S) * std::sin(PI * r * R);

            Vec3 norm(x, y, z);
            Vec3 pos = center + norm * radius;
            Vec2 uv(s * S, r * R);
            Vec3 tang(-std::sin(2.0f * PI * s * S), 0.0f, std::cos(2.0f * PI * s * S));

            m_vertices.push_back({pos, norm, uv, tang, color});
        }
    }

    for (int r = 0; r < rings - 1; ++r) {
        for (int s = 0; s < sectors - 1; ++s) {
            uint32_t i0 = baseIdx + r * sectors + s;
            uint32_t i1 = baseIdx + (r + 1) * sectors + s;
            uint32_t i2 = baseIdx + (r + 1) * sectors + (s + 1);
            uint32_t i3 = baseIdx + r * sectors + (s + 1);

            m_indices.push_back(i0);
            m_indices.push_back(i1);
            m_indices.push_back(i2);

            m_indices.push_back(i0);
            m_indices.push_back(i2);
            m_indices.push_back(i3);
        }
    }
}

void MeshBuilder::build(QOpenGLFunctions_3_3_Core* gl, Mesh& outMesh) const {
    outMesh.upload(gl, m_vertices, m_indices);
}

} // namespace hm
