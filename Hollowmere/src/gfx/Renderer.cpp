#include "Renderer.h"
#include "ProcTex.h"
#include "MeshBuilder.h"
#include "world/wings/Wings.h"
#include <algorithm>
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

    QImage whiteImg(4, 4, QImage::Format_RGBA8888);
    whiteImg.fill(Qt::white);
    m_texWhite.createFromImage(gl, whiteImg);

    QImage bloodImg = ProcTex::generateBloodSplatter(512, 505);
    m_texBlood.createFromImage(gl, bloodImg);

    // 4. Build dynamic prop meshes
    MeshBuilder mb;

    // Doors: hinge at the origin, leaf runs along +X for one full cell, coloured lock plate per key
    static const Vec4 plate[5] = {Vec4(0.75f, 0.62f, 0.25f, 1), Vec4(0.25f, 0.8f, 0.35f, 1), Vec4(0.75f, 0.28f, 0.12f, 1), Vec4(0.95f, 0.55f, 0.7f, 1), Vec4(0.9f, 0.8f, 0.3f, 1)};
    for (int k = 0; k < 5; ++k) {
        mb.clear();
        mb.addBox(Vec3(1.2f, 1.28f, 0.0f), Vec3(2.36f, 2.56f, 0.1f), Vec2(1, 1), Vec4(0.55f, 0.37f, 0.22f, 1));
        mb.addBox(Vec3(1.2f, 1.28f, 0.0f), Vec3(1.9f, 2.1f, 0.13f), Vec2(1, 1), Vec4(0.42f, 0.27f, 0.15f, 1));
        mb.addBox(Vec3(2.1f, 1.05f, 0.0f), Vec3(0.22f, 0.3f, 0.16f), Vec2(1, 1), plate[k]);
        mb.addCylinder(Vec3(2.0f, 1.0f, 0.0f), 0.04f, 0.2f, 8, Vec4(0.85f, 0.75f, 0.35f, 1));
        mb.build(gl, m_doorMeshes[k]);
    }

    // Pickups. Sigils glow gold-white against each wing's colour; keys are tagged by colour.
    static const Vec4 keyCol[5] = {Vec4(1, 1, 1, 1), Vec4(0.3f, 0.9f, 0.4f, 1), Vec4(0.85f, 0.3f, 0.15f, 1), Vec4(1.0f, 0.6f, 0.75f, 1), Vec4(0.95f, 0.85f, 0.3f, 1)};
    for (int k = 1; k <= 4; ++k) {
        mb.clear();
        mb.addSphere(Vec3(-0.12f, 0, 0), 0.07f, 6, 8, Vec4(0.9f, 0.8f, 0.4f, 1));
        mb.addBox(Vec3(0.05f, 0, 0), Vec3(0.3f, 0.035f, 0.035f), Vec2(1, 1), Vec4(0.9f, 0.8f, 0.4f, 1));
        mb.addBox(Vec3(0.14f, -0.04f, 0), Vec3(0.035f, 0.07f, 0.035f), Vec2(1, 1), Vec4(0.9f, 0.8f, 0.4f, 1));
        mb.addBox(Vec3(0.07f, -0.04f, 0), Vec3(0.035f, 0.07f, 0.035f), Vec2(1, 1), Vec4(0.9f, 0.8f, 0.4f, 1));
        mb.addBox(Vec3(-0.12f, 0.0f, 0), Vec3(0.05f, 0.05f, 0.1f), Vec2(1, 1), keyCol[k]);
        mb.build(gl, m_keyMeshes[k]);
    }
    {   // 1: Ada's ribbon (crimson bow)
        mb.clear();
        mb.addBox(Vec3(-0.12f, 0, 0), Vec3(0.2f, 0.03f, 0.12f), Vec2(1, 1), Vec4(0.9f, 0.1f, 0.15f, 1));
        mb.addBox(Vec3(0.12f, 0, 0), Vec3(0.2f, 0.03f, 0.12f), Vec2(1, 1), Vec4(0.9f, 0.1f, 0.15f, 1));
        mb.addSphere(Vec3(0, 0, 0), 0.06f, 6, 8, Vec4(1.0f, 0.3f, 0.3f, 1));
        mb.addBox(Vec3(0.0f, -0.02f, 0.12f), Vec3(0.05f, 0.02f, 0.22f), Vec2(1, 1), Vec4(0.8f, 0.08f, 0.12f, 1));
        mb.build(gl, m_sigilMeshes[1]);
        // 2: tooth ring
        mb.clear();
        mb.addCylinder(Vec3(0, -0.015f, 0), 0.17f, 0.03f, 14, Vec4(0.75f, 0.75f, 0.8f, 1));
        mb.addCylinder(Vec3(0, -0.012f, 0), 0.135f, 0.04f, 14, Vec4(0.02f, 0.02f, 0.02f, 1));
        mb.addSphere(Vec3(0, 0.04f, 0.17f), 0.07f, 6, 8, Vec4(1.0f, 0.97f, 0.88f, 1));
        mb.build(gl, m_sigilMeshes[2]);
        // 3: bent tuning fork
        mb.clear();
        mb.addBox(Vec3(0, -0.12f, 0), Vec3(0.04f, 0.24f, 0.04f), Vec2(1, 1), Vec4(0.7f, 0.8f, 0.95f, 1));
        mb.addBox(Vec3(-0.04f, 0.1f, 0), Vec3(0.03f, 0.26f, 0.03f), Vec2(1, 1), Vec4(0.8f, 0.9f, 1.0f, 1));
        mb.addBox(Vec3(0.05f, 0.1f, 0), Vec3(0.03f, 0.26f, 0.03f), Vec2(1, 1), Vec4(0.8f, 0.9f, 1.0f, 1));
        mb.addSphere(Vec3(0.0f, 0.0f, 0), 0.05f, 6, 8, Vec4(0.9f, 0.95f, 1.0f, 1));
        mb.build(gl, m_sigilMeshes[3]);
        // 4: Eli's blanket
        mb.clear();
        mb.addBox(Vec3(0, 0, 0), Vec3(0.34f, 0.07f, 0.28f), Vec2(1, 1), Vec4(0.95f, 0.65f, 0.78f, 1));
        mb.addBox(Vec3(0, 0.05f, 0), Vec3(0.26f, 0.05f, 0.2f), Vec2(1, 1), Vec4(1.0f, 0.82f, 0.88f, 1));
        mb.addSphere(Vec3(0.1f, 0.1f, 0), 0.05f, 6, 8, Vec4(1.0f, 0.9f, 0.5f, 1));
        mb.build(gl, m_sigilMeshes[4]);
    }
    mb.clear();   // note: a pale folded page
    mb.addBox(Vec3(0, 0, 0), Vec3(0.3f, 0.02f, 0.4f), Vec2(1, 1), Vec4(0.95f, 0.92f, 0.78f, 1));
    mb.addBox(Vec3(0.02f, 0.015f, 0.02f), Vec3(0.24f, 0.01f, 0.3f), Vec2(1, 1), Vec4(0.85f, 0.8f, 0.62f, 1));
    mb.build(gl, m_noteMesh);
    mb.clear();   // oil flask
    mb.addCylinder(Vec3(0, -0.12f, 0), 0.075f, 0.2f, 10, Vec4(0.9f, 0.55f, 0.1f, 1));
    mb.addCylinder(Vec3(0, 0.08f, 0), 0.03f, 0.08f, 8, Vec4(0.8f, 0.7f, 0.5f, 1));
    mb.build(gl, m_oilMesh);
    mb.clear();   // bandage roll
    mb.addBox(Vec3(0, 0, 0), Vec3(0.28f, 0.1f, 0.16f), Vec2(1, 1), Vec4(0.95f, 0.95f, 0.95f, 1));
    mb.addBox(Vec3(0, 0.055f, 0), Vec3(0.18f, 0.02f, 0.04f), Vec2(1, 1), Vec4(0.9f, 0.1f, 0.1f, 1));
    mb.addBox(Vec3(0, 0.055f, 0), Vec3(0.04f, 0.02f, 0.12f), Vec2(1, 1), Vec4(0.9f, 0.1f, 0.1f, 1));
    mb.build(gl, m_bandageMesh);

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

void Renderer::render(QOpenGLFunctions_3_3_Core* gl, World& world, float totalTime, GLuint targetFbo) {
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

    // Per-wing atmosphere, eased so crossing a doorway doesn't pop
    {
        float dt = std::clamp(totalTime - m_lastTime, 0.0f, 0.1f);
        m_lastTime = totalTime;
        WingDefinition wd = WingsManager::getWing(world.activeWing());
        Vec3 ambTarget = wd.ambientLight * 3.0f + Vec3(0.05f, 0.05f, 0.06f);
        float k = std::min(1.0f, dt * 1.5f);
        m_ambient = lerp(m_ambient, ambTarget, k);
        m_fogColor = lerp(m_fogColor, wd.fogColor * 0.8f, k);
        m_fogDensity = lerp(m_fogDensity, 0.030f + wd.fogDensity * 0.35f, k);
    }
    m_worldShader.setUniformVec3(gl, "uAmbientColor", m_ambient);
    m_worldShader.setUniformVec3(gl, "uFogColor", m_fogColor);
    m_worldShader.setUniformFloat(gl, "uFogDensity", m_fogDensity);

    // Lantern Spotlight setup
    m_worldShader.setUniformVec3(gl, "uSpotLight.position", spot.position);
    m_worldShader.setUniformVec3(gl, "uSpotLight.direction", spot.direction);
    m_worldShader.setUniformVec3(gl, "uSpotLight.color", spot.color);
    m_worldShader.setUniformFloat(gl, "uSpotLight.intensity", spot.intensity * 1.35f);
    m_worldShader.setUniformFloat(gl, "uSpotLight.innerCutoff", spot.innerCutoff);
    m_worldShader.setUniformFloat(gl, "uSpotLight.outerCutoff", spot.outerCutoff);
    m_worldShader.setUniformFloat(gl, "uSpotLight.range", spot.range);
    m_worldShader.setUniformInt(gl, "uSpotLight.enabled", spot.enabled ? 1 : 0);

    // Point lights: a faint halo around the player (so the world never goes fully black),
    // a beacon on the current objective, then the nearest wall candles.
    {
        std::vector<PointLight> lights;
        Vec3 pp = world.player().position();
        PointLight aura;
        aura.position = world.player().eyePosition() + Vec3(0, 0.2f, 0);
        aura.color = Vec3(0.85f, 0.75f, 0.65f);
        aura.intensity = world.player().lantern().isOn() ? 0.35f : 0.7f;
        aura.range = 5.0f;
        lights.push_back(aura);
        const ObjectiveInfo& obj = world.objective();
        if (obj.hasTarget && distance(obj.target, pp) < 45.0f) {
            PointLight beacon;
            beacon.position = Vec3(obj.target.x, 1.1f, obj.target.z);
            beacon.color = Vec3(1.0f, 0.82f, 0.35f);
            float close = std::clamp((distance(obj.target, pp) - 2.0f) / 5.0f, 0.25f, 1.0f);   // soften when you are on top of it
            beacon.intensity = (1.2f + 0.3f * std::sin(totalTime * 3.0f)) * close;
            beacon.range = 7.5f;
            lights.push_back(beacon);
        }
        std::vector<std::pair<float, const PointLight*>> near;
        for (const auto& pl : world.map().pointLights) {
            float dx = pl.position.x - pp.x, dz = pl.position.z - pp.z;
            near.push_back({(dx * dx + dz * dz) / (pl.range * pl.range), &pl});   // big lights outrank small candles
        }
        size_t take = std::min<size_t>(near.size(), 12 - lights.size());
        std::partial_sort(near.begin(), near.begin() + take, near.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
        for (size_t i = 0; i < take; ++i) {
            PointLight pl = *near[i].second;
            pl.intensity *= 0.85f + 0.15f * std::sin(totalTime * 7.0f + pl.position.x * 3.1f + pl.position.z);   // candle flicker
            lights.push_back(pl);
        }
        m_worldShader.setUniformInt(gl, "uNumPointLights", static_cast<int>(lights.size()));
        for (size_t i = 0; i < lights.size(); ++i) {
            std::string prefix = "uPointLights[" + std::to_string(i) + "].";
            m_worldShader.setUniformVec3(gl, (prefix + "position").c_str(), lights[i].position);
            m_worldShader.setUniformVec3(gl, (prefix + "color").c_str(), lights[i].color);
            m_worldShader.setUniformFloat(gl, (prefix + "intensity").c_str(), lights[i].intensity);
            m_worldShader.setUniformFloat(gl, (prefix + "range").c_str(), lights[i].range);
            m_worldShader.setUniformInt(gl, (prefix + "enabled").c_str(), 1);
        }
    }

    // 3. Render Geometry
    renderWorld(gl, world);
    renderDoors(gl, world);
    renderPickups(gl, world);
    renderGhosts(gl, world);
    renderLanternModel(gl, world);

    m_worldShader.unbind(gl);

    // 4. End FBO & Run Post-Processing Pass to Screen target FBO
    m_postFx.end(gl, targetFbo);

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
    m_texFloor.bind(gl, 0);      // dark timber beams read far better than the blotchy plaster
    m_normFloor.bind(gl, 1);
    world.map().ceilingMesh.draw(gl);

    // Props
    m_worldShader.setUniformInt(gl, "uUseNormalMap", 0);
    m_texProps.bind(gl, 0);
    world.map().propsMesh.draw(gl);
}

void Renderer::renderDoors(QOpenGLFunctions_3_3_Core* gl, World& world) {
    m_texWhite.bind(gl, 0);
    m_worldShader.setUniformInt(gl, "uUseNormalMap", 0);

    for (const auto& door : world.map().doors) {
        float yaw = door.spansX() ? 0.0f : -90.0f;
        Mat4 model = Mat4::translate(door.hingePosition()) * Mat4::rotateY(radians(yaw + door.openAngle()));
        m_worldShader.setUniformMat4(gl, "uModel", model);
        m_doorMeshes[std::clamp(door.keyId(), 0, 4)].draw(gl);
    }
}

void Renderer::renderPickups(QOpenGLFunctions_3_3_Core* gl, World& world) {
    m_texWhite.bind(gl, 0);
    m_worldShader.setUniformInt(gl, "uUseNormalMap", 0);

    for (const auto& pickup : world.map().pickups) {
        if (pickup.isCollected()) continue;
        const Mesh* mesh = &m_oilMesh;
        switch (pickup.type()) {
        case PickupType::MemorySigil: mesh = &m_sigilMeshes[std::clamp(pickup.tag(), 1, 4)]; break;
        case PickupType::Key: mesh = &m_keyMeshes[std::clamp(pickup.tag(), 1, 4)]; break;
        case PickupType::Note: mesh = &m_noteMesh; break;
        case PickupType::Bandage: mesh = &m_bandageMesh; break;
        case PickupType::OilFlask: mesh = &m_oilMesh; break;
        }
        float scale = pickup.type() == PickupType::MemorySigil ? 1.6f : 1.2f;
        Mat4 model = Mat4::translate(pickup.renderPosition()) * Mat4::rotateY(radians(pickup.rotationAngle()));
        model = model * Mat4::scale(Vec3(scale, scale, scale));
        m_worldShader.setUniformMat4(gl, "uModel", model);
        mesh->draw(gl);
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
        float yaw = door.spansX() ? 0.0f : -90.0f;
        Mat4 model = Mat4::translate(door.hingePosition()) * Mat4::rotateY(radians(yaw + door.openAngle()));
        depthShader.setUniformMat4(gl, "uModel", model);
        m_doorMeshes[std::clamp(door.keyId(), 0, 4)].draw(gl);
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
    m_texWhite.destroy(gl);
    for (auto& m : m_doorMeshes) m.destroy(gl);
    for (auto& m : m_sigilMeshes) m.destroy(gl);
    for (auto& m : m_keyMeshes) m.destroy(gl);
    m_noteMesh.destroy(gl);
    m_oilMesh.destroy(gl);
    m_bandageMesh.destroy(gl);
    m_lanternModelMesh.destroy(gl);
}

} // namespace hm
