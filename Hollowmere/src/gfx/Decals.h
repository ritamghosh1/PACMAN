#pragma once

#include "core/mathx.h"
#include "Mesh.h"
#include <vector>

namespace hm {

enum class DecalType {
    BloodSplatter,
    BloodFootprint,
    BloodyHandprint,
    SootScorch
};

struct DecalInstance {
    DecalType type;
    Vec3 position;
    Vec3 normal;
    float size = 1.0f;
    float opacity = 0.95f;
};

class DecalSystem {
public:
    DecalSystem();

    void addDecal(DecalType type, const Vec3& pos, const Vec3& normal, float size = 1.0f);
    void clear();

    const std::vector<DecalInstance>& decals() const { return m_decals; }

    // Generates a dynamic batched quad mesh for all active decals
    void buildDecalsMesh(QOpenGLFunctions_3_3_Core* gl, Mesh& outMesh) const;

private:
    std::vector<DecalInstance> m_decals;
};

} // namespace hm
