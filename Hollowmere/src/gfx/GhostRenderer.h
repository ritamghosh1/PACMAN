#pragma once

#include "Shader.h"
#include "Mesh.h"
#include "core/mathx.h"
#include "Camera.h"
#include <QOpenGLFunctions_3_3_Core>
#include <memory>

namespace hm {

enum class GhostVisualType {
    Weeper,     // Slender weeping girl, hands to face, trailing shroud
    HollowMan,  // Tall faceless gardener, stiff posture, shears in hand
    Choir,      // 3 fused heads, twisted ribcage
    Doll,       // Small porcelain child with stitched eye-mask
    Warden      // Imposing hulking physician, heavy coat, dragging chain
};

class GhostRenderer {
public:
    GhostRenderer();
    ~GhostRenderer();

    bool init(QOpenGLFunctions_3_3_Core* gl);
    void destroy(QOpenGLFunctions_3_3_Core* gl);

    void render(QOpenGLFunctions_3_3_Core* gl,
                GhostVisualType type,
                const Mat4& modelMatrix,
                const Camera& camera,
                float aspect,
                float timeSec,
                float dissolve = 0.0f,
                float agitation = 0.0f,
                const Vec3& lanternPos = Vec3(0, 0, 0),
                const Vec3& lanternDir = Vec3(0, 0, -1),
                float lanternCutoff = 0.85f,
                bool lanternOn = true);

    void drawShadow(QOpenGLFunctions_3_3_Core* gl,
                    GhostVisualType type,
                    const Mat4& modelMatrix,
                    Shader& depthShader);

private:
    void buildMeshes(QOpenGLFunctions_3_3_Core* gl);
    Mesh* getMesh(GhostVisualType type);

    std::unique_ptr<Shader> m_shader;

    Mesh m_weeperMesh;
    Mesh m_hollowManMesh;
    Mesh m_choirMesh;
    Mesh m_dollMesh;
    Mesh m_wardenMesh;
};

} // namespace hm
