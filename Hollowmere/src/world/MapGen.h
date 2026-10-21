#pragma once

#include <vector>
#include <memory>
#include <QOpenGLFunctions_3_3_Core>
#include "Collision.h"
#include "Door.h"
#include "Pickup.h"
#include "gfx/Mesh.h"
#include "gfx/Light.h"

namespace hm {

struct MapData {
    Mesh wallMesh;
    Mesh floorMesh;
    Mesh ceilingMesh;
    Mesh propsMesh;

    std::vector<Door> doors;
    std::vector<Pickup> pickups;
    std::vector<PointLight> pointLights;
    Collision collision;
    Vec3 playerSpawnPos{0, 0.2f, 0};
};

class MapGen {
public:
    static bool generateManor(QOpenGLFunctions_3_3_Core* gl, MapData& outMap);
};

} // namespace hm
