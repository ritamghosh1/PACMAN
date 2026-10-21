#include "Renderer.h"
#include "ProcTex.h"
#include "MeshBuilder.h"
#include <QDebug>

namespace hm {

Renderer::Renderer() = default;

Renderer::~Renderer() = default;

bool Renderer::init(QOpenGLFunctions_3_3_Core* gl, int width, int height) {
    m_width = width;
    m_height = height;

    gl->glEnable(GL_DEPTH_TEST);
    gl->glDepthFunc(GL_LESS);
    gl->glEnable(GL_CULL_FACE);
    gl->glCullFace(GL_BACK);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Shaders
    if (!m_worldShader.loadFromFiles(gl, ":/shaders/world.vert", ":/shaders/world.frag")) {
        qCritical("Failed to load world shaders!");
        return false;
    }

    // 2. PostFx & ShadowMap
    if (!m_postFx.init(gl, width, height)) {
        qCritical("Failed to initialize PostFx!");
        return false;
    }
    if (!m_shadowMap.init(gl, 1024)) {
        qWarning("Failed to initialize ShadowMap!");
    }

    // 3. Generate Procedural Textures & Normal Maps
    qInfo("Generating procedural horror textures...");
    QImage stoneImg = ProcTex::generateStoneBrick(512, 42);
    QImage stoneNorm = ProcTex::generateNormalMap(stoneImg, 3.0f);
    m_texWall.createFromImage(gl, stoneImg);
    m_normWall.createFromImage(gl, stoneNorm);

    QImage woodImg = ProcTex::generateWoodPlanks(512, 101);
    QImage woodNorm = ProcTex::generateNormalMap(woodImg, 2.2f);
    m_texFloor.createFromImage(gl, woodImg);
    m_normFloor.createFromImage(gl, woodNorm);

    QImage plasterImg = ProcTex::generatePeelingPlaster(512, 202);
    QImage plasterNorm = ProcTex::generateNormalMap(plasterImg, 2.5f);
    m_texCeil.createFromImage(gl, plasterImg);
    m_normCeil.createFromImage(gl, plasterNorm);

    QImage rustImg = ProcTex::generateRustMetal(512, 303);
    m_texProps.createFromImage(gl, rustImg);

    QImage bloodImg = ProcTex::generateBloodSplatter(512, 505);
    m_texBlood.createFromImage(gl, bloodImg);

    // 4. Build dynamic prop meshes
    MeshBuilder mb;

    // Door Mesh (width 1.7m, height 2.35m, depth 0.08m)
    mb.clear();
    mb.addBox(Vec3(0.85f, 1.175f, 0.0f), Vec3(1.7f, 2.35f, 0.08f), Vec2(1, 1), Vec4(0.5f, 0.35f, 0.2f, 1));
    // Door handle & lock
    mb.addCylinder(Vec3(1.55f, 1.05f, 0.0f), 0.045f, 0.14f, 8, Vec4(0.8f, 0.7f, 0.3f, 1));
    mb.build(gl, m_doorMesh);

    // Pickup Mesh (Floating Sigil / Relic Diamond)
    mb.clear();
    mb.addSphere(Vec3(0, 0, 0), 0.16f, 8, 8, Vec4(1.0f, 0.85f, 0.25f, 1));
    mb.addBox(Vec3(0, 0, 0), Vec3(0.24f, 0.24f, 0.24f), Vec2(1, 1), Vec4(0.9f, 0.2f, 0.2f, 0.85f));
    mb.build(gl, m_pickupMesh);

    // Held Lantern Model Mesh
    mb.clear();
    // Metal base and cap
    mb.addCylinder(Vec3(0, -0.18f, 0), 0.09f, 0.05f, 10, Vec4(0.25f, 0.25f, 0.25f, 1));
    mb.addCylinder(Vec3(0,  0.14f, 0), 0.08f, 0.06f, 10, Vec4(0.25f, 0.25f, 0.25f, 1));
    // Glass cage cylinders
    mb.addCylinder(Vec3(0, -0.13f, 0), 0.07f, 0.27f, 10, Vec4(1.0f, 0.95f, 0.8f, 0.4f));
    // Inner glowing flame core
    mb.addSphere(Vec3(0, 0.01f, 0), 0.04f, 8, 8, Vec4(1.0f, 0.8f, 0.2f, 1.0f));
    // Metal handle
    mb.addCylinder(Vec3(0, 0.20f, 0), 0.015f, 0.10f, 6, Vec4(0.3f, 0.3f, 0.3f, 1));
    mb.build(gl, m_lanternModelMesh);

    // 5. Initialize custom Ghost Renderer
    m_ghostRenderer.init(gl);

    return true;
}

void Renderer::resize(QOpenGLFunctions_3_3_Core* gl, int width, int height) {
    if (width <= 0 || height <= 0) return;
    m_width = width;
    m_height = height;
    m_postFx.resize(gl, width, height);
}

void Renderer::render(QOpenGLFunctions_3_3_Core* gl, World& world, float totalTime) {
    // 1. Update Camera
    m_camera.setPosition(world.player().eyePosition());
    m_camera.setRotation(world.player().yaw(), world.player().pitch());

    SpotLight spot = world.player().lantern().spotLight();

    // 2. Render Shadow Depth Pass (for lantern spotlight)
    if (spot.enabled) {
        renderShadowPass(gl, world, spot);
    }

    // 3. Begin HDR Scene Render into PostFx FBO
    m_postFx.begin(gl);

    gl->glEnable(GL_DEPTH_TEST);
    gl->glEnable(GL_CULL_FACE);

    m_worldShader.bind(gl);

    float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    Mat4 view = m_camera.viewMatrix();
    Mat4 proj = m_camera.projMatrix(aspect);

    m_worldShader.setUniformMat4(gl, "uView", view);
    m_worldShader.setUniformMat4(gl, "uProj", proj);
    m_worldShader.setUniformVec3(gl, "uViewPos", m_camera.position());

    // Shadows
    m_worldShader.setUniformMat4(gl, "uLightSpaceMatrix", m_shadowMap.lightSpaceMatrix());
    m_worldShader.setUniformInt(gl, "uShadowMap", 2);
    m_worldShader.setUniformInt(gl, "uUseShadows", spot.enabled ? 1 : 0);
    m_shadowMap.bindDepthTexture(gl, 2);

    // Environmental cold gloom ambient
    m_worldShader.setUniformVec3(gl, "uAmbientColor", Vec3(0.045f, 0.055f, 0.075f));

    // Fog: thick cold asylum atmosphere
    m_worldShader.setUniformVec3(gl, "uFogColor", Vec3(0.018f, 0.022f, 0.035f));
    m_worldShader.setUniformFloat(gl, "uFogDensity", 0.085f);

    // Lantern Spotlight setup
    m_worldShader.setUniformVec3(gl, "uSpotLight.position", spot.position);
    m_worldShader.setUniformVec3(gl, "uSpotLight.direction", spot.direction);
    m_worldShader.setUniformVec3(gl, "uSpotLight.color", spot.color);
    m_worldShader.setUniformFloat(gl, "uSpotLight.intensity", spot.intensity);
    m_worldShader.setUniformFloat(gl, "uSpotLight.innerCutoff", spot.innerCutoff);
    m_worldShader.setUniformFloat(gl, "uSpotLight.outerCutoff", spot.outerCutoff);
    m_worldShader.setUniformFloat(gl, "uSpotLight.range", spot.range);
    m_worldShader.setUniformInt(gl, "uSpotLight.enabled", spot.enabled ? 1 : 0);

    // Point lights (candles/lamps)
    const auto& pLights = world.map().pointLights;
    int numPL = static_cast<int>(std::min(pLights.size(), size_t(4)));
    m_worldShader.setUniformInt(gl, "uNumPointLights", numPL);
    for (int i = 0; i < numPL; ++i) {
        std::string prefix = "uPointLights[" + std::to_string(i) + "].";
        m_worldShader.setUniformVec3(gl, (prefix + "position").c_str(), pLights[i].position);
        m_worldShader.setUniformVec3(gl, (prefix + "color").c_str(), pLights[i].color);
        m_worldShader.setUniformFloat(gl, (prefix + "intensity").c_str(), pLights[i].intensity);
        m_worldShader.setUniformFloat(gl, (prefix + "range").c_str(), pLights[i].range);
        m_worldShader.setUniformInt(gl, (prefix + "enabled").c_str(), pLights[i].enabled ? 1 : 0);
    }

    // 3. Render Geometry
    renderWorld(gl, world);
    renderDoors(gl, world);
    renderPickups(gl, world);
    renderGhosts(gl, world);
    renderLanternModel(gl, world);

    m_worldShader.unbind(gl);

    // 4. End FBO & Run Post-Processing Pass to Screen
    m_postFx.end(gl);

    gl->glViewport(0, 0, m_width, m_height);
    m_postFx.render(gl, totalTime,
                    world.fearLevel(),
                    world.player().healthNormalized(),
                    world.player().sanityNormalized());
}

void Renderer::renderWorld(QOpenGLFunctions_3_3_Core* gl, World& world) {
    Mat4 identity = Mat4::identity();
    m_worldShader.setUniformMat4(gl, "uModel", identity);

    m_worldShader.setUniformInt(gl, "uDiffuseMap", 0);
    m_worldShader.setUniformInt(gl, "uNormalMap", 1);
    m_worldShader.setUniformInt(gl, "uUseNormalMap", 1);

    // Walls
    m_texWall.bind(gl, 0);
    m_normWall.bind(gl, 1);
    world.map().wallMesh.draw(gl);

    // Floor
    m_texFloor.bind(gl, 0);
    m_normFloor.bind(gl, 1);
    world.map().floorMesh.draw(gl);

    // Ceiling
    m_texCeil.bind(gl, 0);
    m_normCeil.bind(gl, 1);
    world.map().ceilingMesh.draw(gl);

    // Props
    m_worldShader.setUniformInt(gl, "uUseNormalMap", 0);
    m_texProps.bind(gl, 0);
    world.map().propsMesh.draw(gl);
}

void Renderer::renderDoors(QOpenGLFunctions_3_3_Core* gl, World& world) {
    m_texProps.bind(gl, 0);
    m_worldShader.setUniformInt(gl, "uUseNormalMap", 0);

    for (const auto& door : world.map().doors) {
        Mat4 model = Mat4::translate(door.position()) * Mat4::rotateY(radians(door.openAngle()));
        m_worldShader.setUniformMat4(gl, "uModel", model);
        m_doorMesh.draw(gl);
    }
}

void Renderer::renderPickups(QOpenGLFunctions_3_3_Core* gl, World& world) {
    m_texProps.bind(gl, 0);
    m_worldShader.setUniformInt(gl, "uUseNormalMap", 0);

    for (const auto& pickup : world.map().pickups) {
        if (pickup.isCollected()) continue;

        Mat4 model = Mat4::translate(pickup.renderPosition()) * Mat4::rotateY(radians(pickup.rotationAngle()));
        m_worldShader.setUniformMat4(gl, "uModel", model);
        m_pickupMesh.draw(gl);
    }
}

void Renderer::renderGhosts(QOpenGLFunctions_3_3_Core* gl, World& world) {
    float aspect = static_cast<float>(m_width) / static_cast<float>(std::max(1, m_height));
    for (const auto& ghost : world.ghosts()) {
        m_ghostRenderer.render(gl,
                               ghost->visualType(),
                               ghost->modelMatrix(),
                               m_camera,
                               aspect,
                               world.elapsedTime(),
                               ghost->dissolve(),
                               ghost->agitation(),
                               world.player().lantern().position(),
                               world.player().lantern().direction(),
                               world.player().lantern().cutoff(),
                               world.player().lantern().isOn());
    }
}

void Renderer::renderLanternModel(QOpenGLFunctions_3_3_Core* gl, World& world) {
    if (!world.player().lantern().isOn()) return;

    Vec3 lanternPos = world.player().lantern().heldPosition();
    Mat4 model = Mat4::translate(lanternPos) * Mat4::rotateY(radians(-world.player().yaw() + 90.0f));

    m_worldShader.setUniformMat4(gl, "uModel", model);
    m_texProps.bind(gl, 0);
    m_worldShader.setUniformInt(gl, "uUseNormalMap", 0);

    m_lanternModelMesh.draw(gl);
}

void Renderer::renderShadowPass(QOpenGLFunctions_3_3_Core* gl, World& world, const SpotLight& spot) {
    m_shadowMap.begin(gl, spot.position, spot.direction, 35.0f, spot.range);

    Shader& depthShader = m_shadowMap.depthShader();
    Mat4 identity = Mat4::identity();
    depthShader.setUniformMat4(gl, "uModel", identity);

    world.map().wallMesh.draw(gl);
    world.map().floorMesh.draw(gl);
    world.map().propsMesh.draw(gl);

    for (const auto& door : world.map().doors) {
        Mat4 model = Mat4::translate(door.position()) * Mat4::rotateY(radians(door.openAngle()));
        depthShader.setUniformMat4(gl, "uModel", model);
        m_doorMesh.draw(gl);
    }

    for (const auto& ghost : world.ghosts()) {
        m_ghostRenderer.drawShadow(gl, ghost->visualType(), ghost->modelMatrix(), depthShader);
    }

    m_shadowMap.end(gl);
}

void Renderer::destroy(QOpenGLFunctions_3_3_Core* gl) {
    m_ghostRenderer.destroy(gl);
    m_shadowMap.destroy(gl);
    m_postFx.destroy(gl);
    m_texWall.destroy(gl);
    m_normWall.destroy(gl);
    m_texFloor.destroy(gl);
    m_normFloor.destroy(gl);
    m_texCeil.destroy(gl);
    m_normCeil.destroy(gl);
    m_texProps.destroy(gl);
    m_texBlood.destroy(gl);
    m_doorMesh.destroy(gl);
    m_pickupMesh.destroy(gl);
    m_lanternModelMesh.destroy(gl);
}

} // namespace hm
