#include "Decals.h"
#include "MeshBuilder.h"

namespace hm {

DecalSystem::DecalSystem() = default;

void DecalSystem::addDecal(DecalType type, const Vec3& pos, const Vec3& normal, float size) {
    if (m_decals.size() >= 128) {
        m_decals.erase(m_decals.begin()); // FIFO eviction
    }
    m_decals.push_back({type, pos + normal * 0.015f, normal, size, 0.95f});
}

void DecalSystem::clear() {
    m_decals.clear();
}

void DecalSystem::buildDecalsMesh(QOpenGLFunctions_3_3_Core* gl, Mesh& outMesh) const {
    MeshBuilder b;

    for (const auto& d : m_decals) {
        Vec3 up = (std::abs(d.normal.y) > 0.9f) ? Vec3(0, 0, 1) : Vec3(0, 1, 0);
        Vec3 right = normalize(cross(d.normal, up)) * (d.size * 0.5f);
        Vec3 forward = normalize(cross(right, d.normal)) * (d.size * 0.5f);

        Vec3 p0 = d.position - right - forward;
        Vec3 p1 = d.position + right - forward;
        Vec3 p2 = d.position + right + forward;
        Vec3 p3 = d.position - right + forward;

        Vec4 color(0.85f, 0.05f, 0.05f, d.opacity);
        if (d.type == DecalType::SootScorch) {
            color = Vec4(0.08f, 0.08f, 0.08f, d.opacity * 0.8f);
        }

        b.addQuad(p0, p1, p2, p3,
                  Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1),
                  d.normal, color);
    }

    b.build(gl, outMesh);
}

} // namespace hm
