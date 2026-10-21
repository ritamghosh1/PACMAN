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

    void render(QOpenGLFunctions_3_3_Core* gl, World& world, float totalTime);
    void destroy(QOpenGLFunctions_3_3_Core* gl);

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
    Texture m_texBlood;

    // Dynamic props meshes
    Mesh m_doorMesh;
    Mesh m_pickupMesh;
    Mesh m_lanternModelMesh;

    int m_width = 1280;
    int m_height = 720;
};

} // namespace hm
