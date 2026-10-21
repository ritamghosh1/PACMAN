#include "PropsGore.h"

namespace hm {

void PropsGore::buildSurgicalGurney(MeshBuilder& b, const Vec3& pos, float yawDeg) {
    (void)yawDeg;
    // 4 rusted metal legs
    Vec4 rustColor(0.35f, 0.22f, 0.18f, 1.0f);
    b.addCylinder(pos + Vec3(-0.85f, 0.0f, -0.35f), 0.035f, 0.75f, 8, rustColor);
    b.addCylinder(pos + Vec3( 0.85f, 0.0f, -0.35f), 0.035f, 0.75f, 8, rustColor);
    b.addCylinder(pos + Vec3(-0.85f, 0.0f,  0.35f), 0.035f, 0.75f, 8, rustColor);
    b.addCylinder(pos + Vec3( 0.85f, 0.0f,  0.35f), 0.035f, 0.75f, 8, rustColor);

    // Stained surgical table top
    Vec4 stainedMetal(0.45f, 0.38f, 0.34f, 1.0f);
    b.addBox(pos + Vec3(0.0f, 0.77f, 0.0f), Vec3(1.9f, 0.06f, 0.85f), Vec2(1, 1), stainedMetal);

    // Blood-soaked canvas pad
    Vec4 bloodPad(0.48f, 0.12f, 0.10f, 1.0f);
    b.addBox(pos + Vec3(0.0f, 0.83f, 0.0f), Vec3(1.7f, 0.06f, 0.75f), Vec2(1, 1), bloodPad);

    // Leather restraint straps across chest and legs
    Vec4 strapColor(0.20f, 0.12f, 0.08f, 1.0f);
    b.addBox(pos + Vec3(-0.25f, 0.87f, 0.0f), Vec3(0.08f, 0.03f, 0.82f), Vec2(1, 1), strapColor);
    b.addBox(pos + Vec3( 0.45f, 0.87f, 0.0f), Vec3(0.08f, 0.03f, 0.82f), Vec2(1, 1), strapColor);
}

void PropsGore::buildSpecimenJar(MeshBuilder& b, const Vec3& pos) {
    // Glass cylinder
    Vec4 glassColor(0.6f, 0.75f, 0.7f, 0.35f);
    b.addCylinder(pos, 0.14f, 0.42f, 12, glassColor);

    // Formaldehyde yellow fluid core
    Vec4 fluidColor(0.7f, 0.65f, 0.3f, 0.5f);
    b.addCylinder(pos + Vec3(0, 0.02f, 0), 0.12f, 0.36f, 10, fluidColor);

    // Preserved organic specimen floating inside
    Vec4 specimenColor(0.55f, 0.22f, 0.20f, 1.0f);
    b.addSphere(pos + Vec3(0, 0.18f, 0), 0.07f, 8, 8, specimenColor);

    // Metal screw cap
    Vec4 capColor(0.3f, 0.3f, 0.3f, 1.0f);
    b.addCylinder(pos + Vec3(0, 0.42f, 0), 0.15f, 0.05f, 12, capColor);
}

void PropsGore::buildRestraintChair(MeshBuilder& b, const Vec3& pos, float yawDeg) {
    (void)yawDeg;
    Vec4 darkWood(0.28f, 0.18f, 0.12f, 1.0f);
    // Legs
    b.addCylinder(pos + Vec3(-0.25f, 0, -0.25f), 0.03f, 0.50f, 8, darkWood);
    b.addCylinder(pos + Vec3( 0.25f, 0, -0.25f), 0.03f, 0.50f, 8, darkWood);
    b.addCylinder(pos + Vec3(-0.25f, 0,  0.25f), 0.03f, 0.50f, 8, darkWood);
    b.addCylinder(pos + Vec3( 0.25f, 0,  0.25f), 0.03f, 0.50f, 8, darkWood);

    // Seat
    b.addBox(pos + Vec3(0, 0.52f, 0), Vec3(0.58f, 0.05f, 0.58f), Vec2(1, 1), darkWood);
    // High backrest
    b.addBox(pos + Vec3(0, 0.95f, -0.26f), Vec3(0.58f, 0.85f, 0.05f), Vec2(1, 1), darkWood);

    // Iron arm shackles
    Vec4 ironShackle(0.2f, 0.2f, 0.22f, 1.0f);
    b.addCylinder(pos + Vec3(-0.30f, 0.75f, 0.10f), 0.04f, 0.08f, 8, ironShackle);
    b.addCylinder(pos + Vec3( 0.30f, 0.75f, 0.10f), 0.04f, 0.08f, 8, ironShackle);
}

void PropsGore::buildHangingHooks(MeshBuilder& b, const Vec3& ceilingPos) {
    Vec4 ironColor(0.25f, 0.25f, 0.25f, 1.0f);
    // Hanging chain
    b.addCylinder(ceilingPos - Vec3(0, 0.8f, 0), 0.015f, 0.8f, 6, ironColor);
    // Curved hook
    b.addCylinder(ceilingPos - Vec3(0, 0.95f, 0), 0.025f, 0.15f, 6, ironColor);
    b.addBox(ceilingPos - Vec3(0.04f, 1.02f, 0), Vec3(0.08f, 0.03f, 0.03f), Vec2(1, 1), ironColor);
}

void PropsGore::buildSewnEyeDummy(MeshBuilder& b, const Vec3& pos, float yawDeg) {
    (void)yawDeg;
    Vec4 burlapColor(0.58f, 0.52f, 0.42f, 1.0f);
    // Head
    b.addSphere(pos + Vec3(0, 1.45f, 0), 0.15f, 10, 10, burlapColor);
    // Black stitched eye line
    Vec4 stitchColor(0.1f, 0.1f, 0.1f, 1.0f);
    b.addBox(pos + Vec3(-0.05f, 1.48f, 0.14f), Vec3(0.04f, 0.01f, 0.01f), Vec2(1, 1), stitchColor);
    b.addBox(pos + Vec3( 0.05f, 1.48f, 0.14f), Vec3(0.04f, 0.01f, 0.01f), Vec2(1, 1), stitchColor);
    // Torso
    b.addCylinder(pos + Vec3(0, 0.85f, 0), 0.18f, 0.65f, 10, burlapColor);
    // Wooden stand
    Vec4 woodStand(0.3f, 0.2f, 0.1f, 1.0f);
    b.addCylinder(pos, 0.03f, 0.85f, 6, woodStand);
    b.addCylinder(pos, 0.22f, 0.04f, 10, woodStand);
}

} // namespace hm
