#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "PostFx.h"
#include "ShadowMap.h"
#include "GhostRenderer.h"
#include "world/World.h"

namespace hm {

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(QOpenGLFunctions_3_3_Core* gl, int width, int height);
    void resize(QOpenGLFunctions_3_3_Core* gl, int width, int height);

    void render(QOpenGLFunctions_3_3_Core* gl, World& world, float totalTime, GLuint targetFbo = 0);
    void destroy(QOpenGLFunctions_3_3_Core* gl);

    // 0 = Low (half-res, no shadows, 4 lights), 1 = Medium, 2 = High (native res, shadows, 12 lights)
    void setQuality(QOpenGLFunctions_3_3_Core* gl, int level);
    int quality() const { return m_quality; }

    Camera& camera() { return m_camera; }
    const Camera& camera() const { return m_camera; }

private:
    void renderWorld(QOpenGLFunctions_3_3_Core* gl, World& world);
    void renderDoors(QOpenGLFunctions_3_3_Core* gl, World& world);
    void renderPickups(QOpenGLFunctions_3_3_Core* gl, World& world);
    void renderGhosts(QOpenGLFunctions_3_3_Core* gl, World& world);
    void renderLanternModel(QOpenGLFunctions_3_3_Core* gl, World& world);
    void renderShadowPass(QOpenGLFunctions_3_3_Core* gl, World& world, const SpotLight& spot);

    Shader m_worldShader;
    PostFx m_postFx;
    ShadowMap m_shadowMap;
    GhostRenderer m_ghostRenderer;
    Camera m_camera;

    // Procedural textures & Normal maps
    Texture m_texWall;
    Texture m_normWall;

    Texture m_texFloor;
    Texture m_normFloor;

    Texture m_texCeil;
    Texture m_normCeil;

    Texture m_texProps;
    Texture m_texWhite;
    Texture m_texBlood;

    // Dynamic props meshes
    Mesh m_doorMeshes[5];          // one per key colour (0 = unlocked door)
    Mesh m_sigilMeshes[5];         // indexed by sigil number 1..4
    Mesh m_keyMeshes[5];           // indexed by key id 1..4
    Mesh m_noteMesh;
    Mesh m_oilMesh;
    Mesh m_bandageMesh;
    Vec3 m_ambient{0.1f, 0.1f, 0.14f};
    Vec3 m_fogColor{0.03f, 0.03f, 0.05f};
    float m_fogDensity = 0.05f;
    float m_lastTime = 0.0f;
    Mesh m_lanternModelMesh;

    int m_quality = 1;
    float m_renderScale = 0.75f;
    int m_maxLights = 8;
    bool m_shadowsOn = true;
    int m_shadowRes = 0;
    int m_width = 1280;
    int m_height = 720;
};

} // namespace hm
