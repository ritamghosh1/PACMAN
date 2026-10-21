#include "GhostRenderer.h"
#include "MeshBuilder.h"

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

void GhostRenderer::buildMeshes(QOpenGLFunctions_3_3_Core* gl) {
    // 1. The Weeper (Ada)
    {
        MeshBuilder b;
        b.addSphere(Vec3(0.0f, 1.55f, 0.05f), 0.16f, 12, 12);
        b.addBox(Vec3(-0.06f, 1.52f, 0.18f), Vec3(0.05f, 0.08f, 0.04f));
        b.addBox(Vec3(0.06f, 1.52f, 0.18f), Vec3(0.05f, 0.08f, 0.04f));
        b.addCylinder(Vec3(0.0f, 0.95f, 0.0f), 0.18f, 0.70f, 12);
        b.addCylinder(Vec3(0.0f, 0.35f, 0.0f), 0.32f, 0.70f, 12);
        b.addBox(Vec3(-0.25f, 1.25f, 0.05f), Vec3(0.06f, 0.50f, 0.06f));
        b.addBox(Vec3(0.25f, 1.25f, 0.05f), Vec3(0.06f, 0.50f, 0.06f));
        b.build(gl, m_weeperMesh);
    }

    // 2. The Hollow Man (Mr. Pell)
    {
        MeshBuilder b;
        b.addSphere(Vec3(0.0f, 1.95f, 0.0f), 0.18f, 14, 14);
        b.addBox(Vec3(0.0f, 1.35f, 0.0f), Vec3(0.36f, 0.85f, 0.22f));
        b.addBox(Vec3(-0.12f, 0.50f, 0.0f), Vec3(0.12f, 0.95f, 0.14f));
        b.addBox(Vec3(0.12f, 0.50f, 0.0f), Vec3(0.12f, 0.95f, 0.14f));
        b.addBox(Vec3(0.28f, 1.30f, 0.15f), Vec3(0.08f, 0.65f, 0.08f));
        b.addBox(Vec3(0.28f, 0.85f, 0.30f), Vec3(0.02f, 0.25f, 0.06f));
        b.addBox(Vec3(0.30f, 0.85f, 0.28f), Vec3(0.02f, 0.25f, 0.06f));
        b.build(gl, m_hollowManMesh);
    }

    // 3. The Choir (3 heads)
    {
        MeshBuilder b;
        b.addSphere(Vec3(0.0f, 1.65f, 0.0f), 0.15f, 12, 12);
        b.addSphere(Vec3(-0.22f, 1.60f, 0.02f), 0.14f, 12, 12);
        b.addSphere(Vec3(0.22f, 1.60f, 0.02f), 0.14f, 12, 12);
        b.addBox(Vec3(0.0f, 1.42f, 0.0f), Vec3(0.55f, 0.16f, 0.20f));
        b.addCylinder(Vec3(0.0f, 0.90f, 0.0f), 0.28f, 0.80f, 14);
        b.addBox(Vec3(-0.35f, 1.05f, 0.10f), Vec3(0.08f, 0.60f, 0.08f));
        b.addBox(Vec3(0.35f, 1.05f, 0.10f), Vec3(0.08f, 0.60f, 0.08f));
        b.addBox(Vec3(-0.15f, 0.95f, 0.25f), Vec3(0.06f, 0.45f, 0.06f));
        b.addCylinder(Vec3(0.0f, 0.25f, 0.0f), 0.38f, 0.60f, 12);
        b.build(gl, m_choirMesh);
    }

    // 4. The Doll (Eli)
    {
        MeshBuilder b;
        b.addSphere(Vec3(0.0f, 0.85f, 0.0f), 0.16f, 14, 14);
        b.addBox(Vec3(0.0f, 0.52f, 0.0f), Vec3(0.22f, 0.40f, 0.16f));
        b.addBox(Vec3(-0.18f, 0.50f, 0.0f), Vec3(0.05f, 0.35f, 0.05f));
        b.addBox(Vec3(0.18f, 0.50f, 0.0f), Vec3(0.05f, 0.35f, 0.05f));
        b.addBox(Vec3(-0.07f, 0.18f, 0.0f), Vec3(0.06f, 0.36f, 0.06f));
        b.addBox(Vec3(0.07f, 0.18f, 0.0f), Vec3(0.06f, 0.36f, 0.06f));
        b.build(gl, m_dollMesh);
    }

    // 5. The Warden (Dr. Crane)
    {
        MeshBuilder b;
        b.addBox(Vec3(0.0f, 2.15f, 0.0f), Vec3(0.22f, 0.26f, 0.22f));
        b.addBox(Vec3(0.0f, 1.55f, 0.0f), Vec3(0.60f, 0.90f, 0.35f));
        b.addCylinder(Vec3(0.0f, 0.55f, 0.0f), 0.45f, 1.10f, 14);
        b.addBox(Vec3(-0.42f, 1.45f, 0.05f), Vec3(0.14f, 0.80f, 0.14f));
        b.addBox(Vec3(0.42f, 1.45f, 0.05f), Vec3(0.14f, 0.80f, 0.14f));
        b.addBox(Vec3(-0.30f, 1.05f, 0.18f), Vec3(0.08f, 0.15f, 0.08f));
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
