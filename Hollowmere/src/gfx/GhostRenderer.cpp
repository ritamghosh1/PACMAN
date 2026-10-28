#include "GhostRenderer.h"
#include "MeshBuilder.h"
#include <cmath>
#include <vector>

namespace hm {

GhostRenderer::GhostRenderer() = default;
GhostRenderer::~GhostRenderer() = default;

bool GhostRenderer::init(QOpenGLFunctions_3_3_Core* gl) {
    m_shader = std::make_unique<Shader>();
    if (!m_shader->loadFromFiles(gl, ":/shaders/ghost.vert", ":/shaders/ghost.frag")) {
        return false;
    }
    buildMeshes(gl);
    return true;
}

void GhostRenderer::destroy(QOpenGLFunctions_3_3_Core* gl) {
    m_weeperMesh.destroy(gl);
    m_hollowManMesh.destroy(gl);
    m_choirMesh.destroy(gl);
    m_dollMesh.destroy(gl);
    m_wardenMesh.destroy(gl);
}

namespace {
using V = std::vector<Vec2>;
const Vec4 kPale(1, 1, 1, 1), kDark(0.04f, 0.04f, 0.05f, 1), kHair(0.10f, 0.10f, 0.12f, 1);
const Vec4 kRibbon(0.95f, 0.10f, 0.15f, 1), kBone(0.94f, 0.92f, 0.86f, 1), kIron(0.30f, 0.30f, 0.34f, 1), kChar(0.16f, 0.12f, 0.10f, 1);

// A child-sized figure in a long gown: used by the Weeper and each member of the Choir.
void gown(MeshBuilder& b, const Vec3& at, float scale, const Vec4& tone) {
    V profile = {{0.30f * scale, 0.08f * scale}, {0.27f * scale, 0.20f * scale}, {0.22f * scale, 0.45f * scale}, {0.15f * scale, 0.72f * scale}, {0.105f * scale, 0.92f * scale}, {0.12f * scale, 1.02f * scale}, {0.07f * scale, 1.10f * scale}};
    b.addLathe(at, profile, 14, tone, 0.85f);
}
} // namespace

void GhostRenderer::buildMeshes(QOpenGLFunctions_3_3_Core* gl) {
    // 1. The Weeper (Ada): small girl, head bowed, both hands pressed to her face, long dark hair, red ribbon
    {
        MeshBuilder b;
        gown(b, Vec3(0, 0.12f, 0), 1.0f, kPale);
        b.addTube(Vec3(0, 1.20f, 0.0f), Vec3(0, 1.32f, 0.03f), 0.045f, 0.035f, 8, kPale);                        // neck
        b.addEllipsoid(Vec3(0, 1.43f, 0.05f), 0.105f, 0.125f, 0.10f, 12, 14, kPale);                             // bowed head
        b.addEllipsoid(Vec3(0, 1.47f, 0.0f), 0.115f, 0.115f, 0.115f, 10, 12, kHair);                             // hair cap
        for (int i = 0; i < 9; ++i) {                                                                             // long wet hair falling over the shoulders
            float x = (i - 4) * 0.026f;
            b.addTube(Vec3(x, 1.52f, -0.04f), Vec3(x * 1.7f, 0.55f - (i % 3) * 0.08f, -0.13f - (i % 2) * 0.03f), 0.022f, 0.004f, 5, kHair);
        }
        for (int side = -1; side <= 1; side += 2) {                                                               // arms: shoulder -> elbow -> hands over face
            b.addTube(Vec3(side * 0.10f, 1.16f, 0.0f), Vec3(side * 0.20f, 1.00f, 0.10f), 0.032f, 0.026f, 6, kPale);
            b.addTube(Vec3(side * 0.20f, 1.00f, 0.10f), Vec3(side * 0.05f, 1.40f, 0.14f), 0.026f, 0.020f, 6, kPale);
            b.addEllipsoid(Vec3(side * 0.045f, 1.43f, 0.145f), 0.035f, 0.05f, 0.025f, 6, 8, kPale);               // hand
        }
        b.addEllipsoid(Vec3(-0.12f, 1.52f, 0.04f), 0.035f, 0.02f, 0.02f, 6, 8, kRibbon);                         // ribbon bow
        b.addEllipsoid(Vec3(-0.17f, 1.50f, 0.04f), 0.03f, 0.018f, 0.02f, 6, 8, kRibbon);
        b.build(gl, m_weeperMesh);
    }

    // 2. The Hollow Man (Mr. Pell): very tall, stooped, long coat, long arms, blank sack face, shears
    {
        MeshBuilder b;
        V coat = {{0.34f, 0.05f}, {0.30f, 0.50f}, {0.27f, 1.00f}, {0.24f, 1.40f}, {0.20f, 1.62f}, {0.12f, 1.74f}};
        b.addLathe(Vec3(0, 0.10f, 0), coat, 12, Vec4(0.32f, 0.34f, 0.30f, 1), 0.7f);
        b.addEllipsoid(Vec3(0, 1.80f, 0.04f), 0.26f, 0.12f, 0.14f, 8, 12, Vec4(0.36f, 0.38f, 0.34f, 1));       // hunched shoulders
        b.addTube(Vec3(0, 1.86f, 0.04f), Vec3(0.0f, 2.00f, 0.12f), 0.05f, 0.04f, 8, kBone);                      // neck
        b.addEllipsoid(Vec3(0, 2.14f, 0.14f), 0.115f, 0.16f, 0.12f, 12, 14, kBone);                              // blank, featureless head
        for (int side = -1; side <= 1; side += 2) {                                                              // arms hanging almost to the knees
            b.addTube(Vec3(side * 0.26f, 1.76f, 0.04f), Vec3(side * 0.34f, 1.00f, 0.10f), 0.045f, 0.035f, 7, Vec4(0.32f, 0.34f, 0.30f, 1));
            b.addTube(Vec3(side * 0.34f, 1.00f, 0.10f), Vec3(side * 0.36f, 0.45f, 0.22f), 0.035f, 0.025f, 7, kBone);
        }
        b.addTube(Vec3(0.36f, 0.45f, 0.22f), Vec3(0.40f, 0.98f, 0.46f), 0.016f, 0.008f, 5, kIron);               // garden shears, blades apart
        b.addTube(Vec3(0.36f, 0.45f, 0.22f), Vec3(0.30f, 0.96f, 0.46f), 0.016f, 0.008f, 5, kIron);
        b.build(gl, m_hollowManMesh);
    }

    // 3. The Choir: three blind children whose throats are joined by one band of flesh
    {
        MeshBuilder b;
        const float xs[3] = {-0.36f, 0.0f, 0.36f}, hs[3] = {0.92f, 1.0f, 0.88f};
        for (int i = 0; i < 3; ++i) {
            float s = hs[i];
            gown(b, Vec3(xs[i], 0.12f, 0), s, kPale);
            b.addTube(Vec3(xs[i], 1.10f * s, 0), Vec3(xs[i], 1.24f * s, 0.0f), 0.04f, 0.035f, 8, kPale);
            b.addEllipsoid(Vec3(xs[i], 1.38f * s, -0.03f), 0.10f, 0.125f, 0.10f, 12, 14, kPale);                 // head tipped back, singing
            b.addEllipsoid(Vec3(xs[i], 1.33f * s, 0.06f), 0.035f, 0.05f, 0.03f, 8, 10, kDark);                   // open mouth
            b.addEllipsoid(Vec3(xs[i] - 0.04f, 1.42f * s, 0.07f), 0.022f, 0.014f, 0.02f, 6, 8, kDark);           // sealed, sunken eyes
            b.addEllipsoid(Vec3(xs[i] + 0.04f, 1.42f * s, 0.07f), 0.022f, 0.014f, 0.02f, 6, 8, kDark);
            b.addTube(Vec3(xs[i] - 0.10f, 1.10f * s, 0.0f), Vec3(xs[i] - 0.14f, 0.55f, 0.05f), 0.03f, 0.022f, 6, kPale);   // arms hang loose
            b.addTube(Vec3(xs[i] + 0.10f, 1.10f * s, 0.0f), Vec3(xs[i] + 0.14f, 0.55f, 0.05f), 0.03f, 0.022f, 6, kPale);
        }
        b.addTube(Vec3(xs[0], 1.17f, 0.02f), Vec3(xs[1], 1.20f, 0.02f), 0.05f, 0.05f, 8, Vec4(0.85f, 0.45f, 0.45f, 1));   // the joined throat
        b.addTube(Vec3(xs[1], 1.20f, 0.02f), Vec3(xs[2], 1.15f, 0.02f), 0.05f, 0.05f, 8, Vec4(0.85f, 0.45f, 0.45f, 1));
        b.build(gl, m_choirMesh);
    }

    // 4. The Doll (Eli): a small boy, oversized porcelain mask with stitched eyes, hugging his blanket
    {
        MeshBuilder b;
        V dress = {{0.20f, 0.06f}, {0.17f, 0.24f}, {0.12f, 0.46f}, {0.095f, 0.60f}, {0.10f, 0.68f}, {0.05f, 0.74f}};
        b.addLathe(Vec3(0, 0.12f, 0), dress, 12, Vec4(0.80f, 0.74f, 0.70f, 1));
        b.addEllipsoid(Vec3(0, 0.98f, 0.02f), 0.145f, 0.17f, 0.14f, 14, 16, kBone);                              // big porcelain head
        b.addBox(Vec3(-0.055f, 1.00f, 0.135f), Vec3(0.07f, 0.012f, 0.012f), Vec2(1, 1), kDark);                  // stitched-shut eyes
        b.addBox(Vec3(0.055f, 1.00f, 0.135f), Vec3(0.07f, 0.012f, 0.012f), Vec2(1, 1), kDark);
        for (int i = -1; i <= 1; ++i) {
            b.addBox(Vec3(-0.055f + i * 0.02f, 0.985f, 0.138f), Vec3(0.006f, 0.04f, 0.008f), Vec2(1, 1), kDark);
            b.addBox(Vec3(0.055f + i * 0.02f, 0.985f, 0.138f), Vec3(0.006f, 0.04f, 0.008f), Vec2(1, 1), kDark);
        }
        b.addBox(Vec3(0, 0.92f, 0.14f), Vec3(0.06f, 0.012f, 0.01f), Vec2(1, 1), Vec4(0.5f, 0.12f, 0.12f, 1));    // painted smile
        b.addTube(Vec3(0.06f, 0.88f, 0.10f), Vec3(0.12f, 0.99f, 0.06f), 0.03f, 0.022f, 6, kBone);                // a crack in the porcelain
        for (int side = -1; side <= 1; side += 2) {
            b.addTube(Vec3(side * 0.08f, 0.80f, 0.0f), Vec3(side * 0.12f, 0.58f, 0.10f), 0.026f, 0.02f, 6, kBone);
            b.addTube(Vec3(side * 0.12f, 0.58f, 0.10f), Vec3(side * 0.03f, 0.55f, 0.16f), 0.02f, 0.018f, 6, kBone);
        }
        b.addBox(Vec3(0, 0.55f, 0.20f), Vec3(0.26f, 0.12f, 0.10f), Vec2(1, 1), Vec4(0.95f, 0.62f, 0.78f, 1));    // blanket held to his chest
        b.build(gl, m_dollMesh);
    }

    // 5. The Warden (Dr. Crane): huge, burnt, the charred ward door fused to his back, iron mask, chains and keys
    {
        MeshBuilder b;
        b.addBox(Vec3(0, 1.20f, -0.26f), Vec3(1.05f, 2.40f, 0.14f), Vec2(1, 1), kChar);                           // the ward door
        b.addBox(Vec3(0, 1.62f, -0.18f), Vec3(0.82f, 0.90f, 0.06f), Vec2(1, 1), Vec4(0.10f, 0.07f, 0.06f, 1));
        b.addBox(Vec3(0, 0.62f, -0.18f), Vec3(0.82f, 0.90f, 0.06f), Vec2(1, 1), Vec4(0.10f, 0.07f, 0.06f, 1));
        V coat = {{0.50f, 0.04f}, {0.46f, 0.55f}, {0.42f, 1.10f}, {0.40f, 1.55f}, {0.30f, 1.80f}, {0.16f, 1.92f}};
        b.addLathe(Vec3(0, 0.05f, 0), coat, 14, Vec4(0.18f, 0.14f, 0.13f, 1), 0.65f);
        b.addEllipsoid(Vec3(0, 1.90f, 0.0f), 0.46f, 0.16f, 0.24f, 8, 14, Vec4(0.22f, 0.17f, 0.15f, 1));         // broad shoulders
        b.addEllipsoid(Vec3(0, 2.18f, 0.03f), 0.14f, 0.18f, 0.14f, 12, 14, Vec4(0.12f, 0.12f, 0.14f, 1));       // iron-masked head
        b.addBox(Vec3(0, 2.18f, 0.17f), Vec3(0.17f, 0.05f, 0.02f), Vec2(1, 1), kDark);                           // eye slit
        b.addBox(Vec3(0, 2.07f, 0.16f), Vec3(0.09f, 0.07f, 0.02f), Vec2(1, 1), kDark);
        for (int side = -1; side <= 1; side += 2) {                                                              // thick arms, hands dragging chains
            b.addTube(Vec3(side * 0.44f, 1.90f, 0.0f), Vec3(side * 0.56f, 1.20f, 0.16f), 0.09f, 0.07f, 8, Vec4(0.20f, 0.15f, 0.14f, 1));
            b.addTube(Vec3(side * 0.56f, 1.20f, 0.16f), Vec3(side * 0.50f, 0.62f, 0.30f), 0.07f, 0.05f, 8, Vec4(0.20f, 0.15f, 0.14f, 1));
            for (int k = 0; k < 7; ++k) {                                                                        // chain links sagging to the floor
                float t = k / 6.0f;
                Vec3 p(side * (0.50f + 0.25f * t), 0.62f - 0.58f * std::sin(t * 1.5707f), 0.30f + 0.5f * t);
                b.addBox(p, Vec3(0.07f, 0.05f, 0.07f), Vec2(1, 1), kIron);
            }
        }
        for (int k = 0; k < 8; ++k) {                                                                            // ring of keys at the hip
            float a = k * 0.785f;
            b.addEllipsoid(Vec3(0.44f + std::cos(a) * 0.07f, 1.00f + std::sin(a) * 0.07f, 0.32f), 0.018f, 0.018f, 0.018f, 5, 6, Vec4(1.0f, 0.78f, 0.25f, 1));
        }
        for (int k = 0; k < 4; ++k) b.addBox(Vec3(0.46f, 0.88f - k * 0.04f, 0.32f), Vec3(0.02f, 0.10f, 0.02f), Vec2(1, 1), Vec4(1.0f, 0.78f, 0.25f, 1));
        b.build(gl, m_wardenMesh);
    }
}

Mesh* GhostRenderer::getMesh(GhostVisualType type) {
    switch (type) {
        case GhostVisualType::Weeper:    return &m_weeperMesh;
        case GhostVisualType::HollowMan: return &m_hollowManMesh;
        case GhostVisualType::Choir:     return &m_choirMesh;
        case GhostVisualType::Doll:      return &m_dollMesh;
        case GhostVisualType::Warden:    return &m_wardenMesh;
    }
    return &m_weeperMesh;
}

void GhostRenderer::render(QOpenGLFunctions_3_3_Core* gl,
                           GhostVisualType type,
                           const Mat4& modelMatrix,
                           const Camera& camera,
                           float aspect,
                           float timeSec,
                           float dissolve,
                           float agitation,
                           const Vec3& lanternPos,
                           const Vec3& lanternDir,
                           float lanternCutoff,
                           bool lanternOn) {
    if (!m_shader || !m_shader->isValid()) return;

    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glDepthMask(GL_FALSE);

    m_shader->bind(gl);
    m_shader->setUniformMat4(gl, "uModel", modelMatrix);
    m_shader->setUniformMat4(gl, "uView", camera.viewMatrix());
    m_shader->setUniformMat4(gl, "uProj", camera.projMatrix(aspect));
    m_shader->setUniformVec3(gl, "uCamPos", camera.position());
    m_shader->setUniformFloat(gl, "uTime", timeSec);
    m_shader->setUniformFloat(gl, "uDissolve", dissolve);
    m_shader->setUniformFloat(gl, "uAgitation", agitation);
    static const Vec3 tint[5] = {Vec3(0.62f, 0.80f, 1.00f), Vec3(0.58f, 0.78f, 0.62f), Vec3(1.00f, 0.84f, 0.62f), Vec3(0.98f, 0.82f, 0.82f), Vec3(0.95f, 0.55f, 0.32f)};
    m_shader->setUniformVec3(gl, "uTint", tint[std::clamp(static_cast<int>(type), 0, 4)]);

    m_shader->setUniformVec3(gl, "uLanternPos", lanternPos);
    m_shader->setUniformVec3(gl, "uLanternDir", lanternDir);
    m_shader->setUniformFloat(gl, "uLanternCutoff", lanternCutoff);
    m_shader->setUniformInt(gl, "uLanternOn", lanternOn ? 1 : 0);

    Mesh* mesh = getMesh(type);
    if (mesh) {
        mesh->draw(gl);
    }

    m_shader->unbind(gl);
    gl->glDepthMask(GL_TRUE);
}

void GhostRenderer::drawShadow(QOpenGLFunctions_3_3_Core* gl,
                               GhostVisualType type,
                               const Mat4& modelMatrix,
                               Shader& depthShader) {
    depthShader.setUniformMat4(gl, "uModel", modelMatrix);
    Mesh* mesh = getMesh(type);
    if (mesh) {
        mesh->draw(gl);
    }
}

} // namespace hm
