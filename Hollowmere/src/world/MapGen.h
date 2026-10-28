#pragma once

#include <string>
#include <vector>
#include <QString>
#include <QOpenGLFunctions_3_3_Core>
#include "Collision.h"
#include "Door.h"
#include "Pickup.h"
#include "gfx/Mesh.h"
#include "gfx/Light.h"

namespace hm {

// One of the six areas of the manor (0 = foyer hub).
struct WingInfo {
    int id = 0;
    QString name;
    int c0 = 0, r0 = 0, c1 = 0, r1 = 0;   // inclusive cell rectangle
    Vec3 entrance;                         // cell just inside the door
    Vec3 safeCenter;
    float safeRadius = 2.6f;
    Vec3 ghostSpawn;
    std::vector<Vec3> patrol;
    int doorId = 0;                        // door leading into this wing (0 = none)
};

struct MapData {
    Mesh wallMesh;
    Mesh floorMesh;
    Mesh ceilingMesh;
    Mesh propsMesh;

    std::vector<Door> doors;
    std::vector<Pickup> pickups;
    std::vector<PointLight> pointLights;
    Collision collision;
    Vec3 playerSpawnPos{0, 0.1f, 0};

    // Layout data used by AI, HUD minimap and objectives
    int gridW = 0, gridH = 0;
    float cellSize = 2.4f;
    Vec3 origin;
    std::vector<std::string> grid;
    std::vector<WingInfo> wings;           // indexed by wing id
    std::vector<int> questOrder;           // pickup ids in the order the story wants them
    Vec3 altarPos;
    Vec3 wardenSpawn;

    Vec3 cellCenter(int c, int r) const;
    bool worldToCell(const Vec3& p, int& c, int& r) const;
    int wingAt(const Vec3& p) const;       // wing id, or -1 inside the maze walls / outside
    const Pickup* findPickup(int id) const;
    const Door* findDoor(int id) const;
};

class MapGen {
public:
    static bool generateManor(QOpenGLFunctions_3_3_Core* gl, MapData& outMap);
};

} // namespace hm
