#pragma once

#include "core/mathx.h"
#include "gfx/MeshBuilder.h"

namespace hm {

class PropsGore {
public:
    static void buildSurgicalGurney(MeshBuilder& b, const Vec3& pos, float yawDeg = 0.0f);
    static void buildSpecimenJar(MeshBuilder& b, const Vec3& pos);
    static void buildRestraintChair(MeshBuilder& b, const Vec3& pos, float yawDeg = 0.0f);
    static void buildHangingHooks(MeshBuilder& b, const Vec3& ceilingPos);
    static void buildSewnEyeDummy(MeshBuilder& b, const Vec3& pos, float yawDeg = 0.0f);
};

} // namespace hm
