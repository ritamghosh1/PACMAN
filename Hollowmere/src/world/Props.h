#pragma once

#include "core/mathx.h"
#include "gfx/MeshBuilder.h"

namespace hm {

class Props {
public:
    static void buildBookshelf(MeshBuilder& b, const Vec3& pos, float yawDeg = 0.0f);
    static void buildGrandfatherClock(MeshBuilder& b, const Vec3& pos, float yawDeg = 0.0f);
    static void buildLocker(MeshBuilder& b, const Vec3& pos, float yawDeg = 0.0f);
    static void buildAltarTable(MeshBuilder& b, const Vec3& pos, float yawDeg = 0.0f);
    static void buildPew(MeshBuilder& b, const Vec3& pos, float yawDeg = 0.0f);
    static void buildMirrorPedestal(MeshBuilder& b, const Vec3& pos, float rotationAngleDeg = 0.0f);
    static void buildSteamPipeRun(MeshBuilder& b, const Vec3& startPos, const Vec3& endPos);
    static void buildCandelabra(MeshBuilder& b, const Vec3& pos);
};

} // namespace hm
