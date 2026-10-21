#include "Props.h"

namespace hm {

void Props::buildBookshelf(MeshBuilder& b, const Vec3& pos, float yawDeg) {
    (void)yawDeg;
    Vec4 woodColor(0.32f, 0.20f, 0.12f, 1.0f);
    // Outer wooden frame
    b.addBox(pos + Vec3(0, 1.3f, 0), Vec3(1.6f, 2.6f, 0.45f), Vec2(1, 1), woodColor);

    // Shelves with books
    Vec4 bookColors[3] = {
        Vec4(0.45f, 0.15f, 0.15f, 1.0f),
        Vec4(0.15f, 0.25f, 0.45f, 1.0f),
        Vec4(0.25f, 0.35f, 0.20f, 1.0f)
    };
    for (int s = 0; s < 4; ++s) {
        float y = 0.5f + s * 0.6f;
        b.addBox(pos + Vec3(-0.35f, y, 0.05f), Vec3(0.55f, 0.35f, 0.30f), Vec2(1, 1), bookColors[s % 3]);
        b.addBox(pos + Vec3( 0.35f, y, 0.05f), Vec3(0.55f, 0.35f, 0.30f), Vec2(1, 1), bookColors[(s + 1) % 3]);
    }
}

void Props::buildGrandfatherClock(MeshBuilder& b, const Vec3& pos, float yawDeg) {
    (void)yawDeg;
    Vec4 mahogany(0.24f, 0.14f, 0.08f, 1.0f);
    // Base & Column
    b.addBox(pos + Vec3(0, 0.25f, 0), Vec3(0.65f, 0.50f, 0.45f), Vec2(1, 1), mahogany);
    b.addBox(pos + Vec3(0, 1.15f, 0), Vec3(0.50f, 1.30f, 0.38f), Vec2(1, 1), mahogany);
    // Clock head
    b.addBox(pos + Vec3(0, 2.10f, 0), Vec3(0.68f, 0.60f, 0.45f), Vec2(1, 1), mahogany);
    // Clock face
    Vec4 clockFace(0.95f, 0.90f, 0.75f, 1.0f);
    b.addCylinder(pos + Vec3(0, 2.10f, 0.20f), 0.20f, 0.05f, 14, clockFace);
}

void Props::buildLocker(MeshBuilder& b, const Vec3& pos, float yawDeg) {
    (void)yawDeg;
    Vec4 greenMetal(0.22f, 0.28f, 0.24f, 1.0f);
    // Locker locker body
    b.addBox(pos + Vec3(0, 1.1f, 0), Vec3(0.70f, 2.2f, 0.65f), Vec2(1, 1), greenMetal);
    // Ventilation louvers
    Vec4 darkVent(0.12f, 0.14f, 0.12f, 1.0f);
    b.addBox(pos + Vec3(0, 1.85f, 0.33f), Vec3(0.40f, 0.15f, 0.02f), Vec2(1, 1), darkVent);
    // Latch handle
    Vec4 brass(0.7f, 0.6f, 0.2f, 1.0f);
    b.addCylinder(pos + Vec3(0.25f, 1.05f, 0.35f), 0.02f, 0.12f, 8, brass);
}

void Props::buildAltarTable(MeshBuilder& b, const Vec3& pos, float yawDeg) {
    (void)yawDeg;
    Vec4 stoneAltar(0.38f, 0.38f, 0.40f, 1.0f);
    // Heavy carved stone plinth
    b.addBox(pos + Vec3(0, 0.25f, 0), Vec3(2.4f, 0.50f, 1.2f), Vec2(1, 1), stoneAltar);
    b.addBox(pos + Vec3(0, 0.85f, 0), Vec3(2.6f, 0.70f, 1.3f), Vec2(1, 1), stoneAltar);
    // 5 Sigil inlays
    Vec4 goldGlow(0.9f, 0.8f, 0.3f, 1.0f);
    for (int i = 0; i < 5; ++i) {
        float x = -0.8f + i * 0.4f;
        b.addCylinder(pos + Vec3(x, 1.21f, 0), 0.08f, 0.02f, 8, goldGlow);
    }
}

void Props::buildPew(MeshBuilder& b, const Vec3& pos, float yawDeg) {
    (void)yawDeg;
    Vec4 darkCedar(0.30f, 0.18f, 0.10f, 1.0f);
    // Seat
    b.addBox(pos + Vec3(0, 0.45f, 0), Vec3(2.8f, 0.06f, 0.55f), Vec2(1, 1), darkCedar);
    // Backrest
    b.addBox(pos + Vec3(0, 0.85f, -0.25f), Vec3(2.8f, 0.75f, 0.06f), Vec2(1, 1), darkCedar);
    // Side panels
    b.addBox(pos + Vec3(-1.4f, 0.55f, 0), Vec3(0.08f, 1.0f, 0.65f), Vec2(1, 1), darkCedar);
    b.addBox(pos + Vec3( 1.4f, 0.55f, 0), Vec3(0.08f, 1.0f, 0.65f), Vec2(1, 1), darkCedar);
}

void Props::buildMirrorPedestal(MeshBuilder& b, const Vec3& pos, float rotationAngleDeg) {
    (void)rotationAngleDeg;
    Vec4 brassBase(0.55f, 0.45f, 0.20f, 1.0f);
    // Ornate fluted pedestal
    b.addCylinder(pos, 0.22f, 0.90f, 12, brassBase);
    // Mirror frame
    b.addBox(pos + Vec3(0, 1.25f, 0), Vec3(0.65f, 0.70f, 0.08f), Vec2(1, 1), brassBase);
    // Mirror glass face
    Vec4 glass(0.85f, 0.92f, 0.95f, 1.0f);
    b.addBox(pos + Vec3(0, 1.25f, 0.045f), Vec3(0.52f, 0.58f, 0.02f), Vec2(1, 1), glass);
}

void Props::buildSteamPipeRun(MeshBuilder& b, const Vec3& startPos, const Vec3& endPos) {
    Vec4 ironPipe(0.28f, 0.30f, 0.32f, 1.0f);
    Vec3 mid = (startPos + endPos) * 0.5f;
    Vec3 diff = endPos - startPos;
    float len = length(diff);
    b.addCylinder(mid - Vec3(0, len * 0.5f, 0), 0.08f, len, 8, ironPipe);
}

void Props::buildCandelabra(MeshBuilder& b, const Vec3& pos) {
    Vec4 wroughtIron(0.2f, 0.2f, 0.2f, 1.0f);
    b.addCylinder(pos, 0.04f, 1.4f, 8, wroughtIron);
    b.addBox(pos + Vec3(0, 1.4f, 0), Vec3(0.6f, 0.04f, 0.04f), Vec2(1, 1), wroughtIron);

    // Candles
    Vec4 wax(0.9f, 0.85f, 0.75f, 1.0f);
    b.addCylinder(pos + Vec3(-0.25f, 1.42f, 0), 0.025f, 0.18f, 6, wax);
    b.addCylinder(pos + Vec3( 0.00f, 1.42f, 0), 0.025f, 0.24f, 6, wax);
    b.addCylinder(pos + Vec3( 0.25f, 1.42f, 0), 0.025f, 0.18f, 6, wax);
}

} // namespace hm
