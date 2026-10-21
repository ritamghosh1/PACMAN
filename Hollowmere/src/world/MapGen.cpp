#include "MapGen.h"
#include "gfx/MeshBuilder.h"
#include "Props.h"
#include "PropsGore.h"

namespace hm {

bool MapGen::generateManor(QOpenGLFunctions_3_3_Core* gl, MapData& outMap) {
    MeshBuilder wallBuilder;
    MeshBuilder floorBuilder;
    MeshBuilder ceilBuilder;
    MeshBuilder propsBuilder;

    outMap.collision.clear();
    outMap.doors.clear();
    outMap.pickups.clear();
    outMap.pointLights.clear();

    // 2D grid template representing Hollowmere Manor & Sanitarium Wings
    // '#' = Stone Wall
    // '.' = Wooden/Tile Floor
    // 'D' = Doorway
    // 'L' = Locked Door (needs key)
    // 'P' = Pillar
    // 'K' = Key Pickup
    // 'O' = Oil Flask Pickup
    // 'B' = Bandage Pickup
    // 'S' = Sigil / Note Pickup
    // '@' = Player Spawn
    // 'C' = Wall Candle / PointLight
    const char* layout[] = {
        "###################################",
        "#.............#...................#",
        "#.###.#######.#.#####.###########.#",
        "#.#...#.....#.#.#...#.#.........#.#",
        "#.#.###.###.#.#.#.#.#.#.#######.#.#",
        "#.#...#...#...#...#.#.#.#.....#.#.#",
        "#.###.###.#######.#.#.#.#.###.#.#.#",
        "#...#...#.......#.#...#...#.#...#.#",
        "###.###.#######.#.#########.#.###.#",
        "#...#.........#.#...........#...#.#",
        "#.###.#######.#.D.#########.###.#.#",
        "#.#.........#.#...#.......#...#.#.#",
        "#.#.#######.#.#####.#####.###.#.#.#",
        "#.#.#.....#.#.......#...#.#...#.#.#",
        "#.#.#.###.#.#########.#.#.#.###.#.#",
        "#...#.#.#.#...........#...#.#...#.#",
        "#####.#.#.#############.###.#.###.#",
        "#C....#.#.....#C....#C....#.#...C.#",
        "#.P...#.#####.#.P..P#.#####.#...P.#",
        "#..@..D.......D.....D.......D..S..#",
        "#.P...#.#####.#.P..P#.#####.#...P.#",
        "#C....#.#...K.#C....#C....O.#.#...#",
        "#####.###.###.#############.###.###",
        "#...#...#.#.#.........#.....#.#...#",
        "#.#.###.#.#.#########.#.###.#.###.#",
        "#.#.#...#.#.........#.#.#.#.#...#.#",
        "#.#.#.###.#########.#.#.#.#.###.#.#",
        "#.#.#.#...........#.#...#.#...#.#.#",
        "#.###.#.#########.#.#####.###.#.#.#",
        "#.....#.#.......#.#.....#...#.#...#",
        "#######.#.#####.#.#####.###.#.###.#",
        "#.......#.#...#.#.....#...#.#.#...#",
        "#.#########.###.#####.###.#.#.#.###",
        "#...........#.......#...#...#...#.#",
        "###################################"
    };

    int mapH = sizeof(layout) / sizeof(layout[0]);
    int mapW = static_cast<int>(std::strlen(layout[0]));

    float cellSize = 2.4f;
    float wallHeight = 3.4f;

    float originX = -(mapW * cellSize) * 0.5f;
    float originZ = -(mapH * cellSize) * 0.5f;

    int doorCounter = 1;
    int pickupCounter = 1;

    for (int r = 0; r < mapH; ++r) {
        for (int c = 0; c < mapW; ++c) {
            char tile = layout[r][c];

            float x = originX + c * cellSize + cellSize * 0.5f;
            float z = originZ + r * cellSize + cellSize * 0.5f;
            Vec3 tileCenter(x, 0.0f, z);

            // Always add Floor and Ceiling (except outermost void)
            if (tile != ' ' ) {
                // Floor
                floorBuilder.addQuad(
                    tileCenter + Vec3(-cellSize * 0.5f, 0.0f,  cellSize * 0.5f),
                    tileCenter + Vec3( cellSize * 0.5f, 0.0f,  cellSize * 0.5f),
                    tileCenter + Vec3( cellSize * 0.5f, 0.0f, -cellSize * 0.5f),
                    tileCenter + Vec3(-cellSize * 0.5f, 0.0f, -cellSize * 0.5f),
                    Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1),
                    Vec3(0, 1, 0)
                );

                // Ceiling
                ceilBuilder.addQuad(
                    tileCenter + Vec3(-cellSize * 0.5f, wallHeight, -cellSize * 0.5f),
                    tileCenter + Vec3( cellSize * 0.5f, wallHeight, -cellSize * 0.5f),
                    tileCenter + Vec3( cellSize * 0.5f, wallHeight,  cellSize * 0.5f),
                    tileCenter + Vec3(-cellSize * 0.5f, wallHeight,  cellSize * 0.5f),
                    Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1),
                    Vec3(0, -1, 0)
                );
            }

            if (tile == '#') {
                // Solid Stone Wall block
                wallBuilder.addBox(
                    Vec3(x, wallHeight * 0.5f, z),
                    Vec3(cellSize, wallHeight, cellSize),
                    Vec2(1.0f, 1.5f)
                );
                outMap.collision.addBox(
                    AABB(Vec3(x - cellSize * 0.5f, 0.0f, z - cellSize * 0.5f),
                         Vec3(x + cellSize * 0.5f, wallHeight, z + cellSize * 0.5f)),
                    0, 0
                );
            } else if (tile == 'P') {
                // Grand Gothic Pillar
                propsBuilder.addCylinder(
                    Vec3(x, 0.0f, z),
                    0.38f, wallHeight, 14,
                    Vec4(0.85f, 0.85f, 0.85f, 1.0f)
                );
                // Pillar base & capital
                propsBuilder.addBox(Vec3(x, 0.2f, z), Vec3(0.9f, 0.4f, 0.9f));
                propsBuilder.addBox(Vec3(x, wallHeight - 0.2f, z), Vec3(0.9f, 0.4f, 0.9f));

                outMap.collision.addBox(
                    AABB(Vec3(x - 0.42f, 0.0f, z - 0.42f),
                         Vec3(x + 0.42f, wallHeight, z + 0.42f)),
                    0, 0
                );
            } else if (tile == 'D' || tile == 'L') {
                bool isLocked = (tile == 'L');
                int dId = doorCounter++;
                Door door(dId, Vec3(x, 0.0f, z), 0.0f, isLocked, isLocked ? 1 : 0);
                outMap.doors.push_back(door);

                // Add door frame trim
                wallBuilder.addBox(Vec3(x - cellSize * 0.5f + 0.15f, wallHeight * 0.5f, z),
                                   Vec3(0.3f, wallHeight, 0.4f));
                wallBuilder.addBox(Vec3(x + cellSize * 0.5f - 0.15f, wallHeight * 0.5f, z),
                                   Vec3(0.3f, wallHeight, 0.4f));
                wallBuilder.addBox(Vec3(x, wallHeight - 0.3f, z),
                                   Vec3(cellSize, 0.6f, 0.4f));

                outMap.collision.addBox(door.currentColliderBox(), 1, dId);
            } else if (tile == '@') {
                outMap.playerSpawnPos = Vec3(x, 0.1f, z);
            } else if (tile == 'O') {
                outMap.pickups.emplace_back(pickupCounter++, PickupType::OilFlask,
                                            Vec3(x, 0.4f, z), "Lantern Oil Flask", "Restores kerosene to your lantern.");
            } else if (tile == 'K') {
                outMap.pickups.emplace_back(pickupCounter++, PickupType::BrassKey,
                                            Vec3(x, 0.4f, z), "Heavy Brass Key", "A rusted key labeled 'Asylum Ward B'.");
            } else if (tile == 'B') {
                outMap.pickups.emplace_back(pickupCounter++, PickupType::Bandage,
                                            Vec3(x, 0.4f, z), "Sterile Bandages", "Stops bleeding and binds wounds.");
            } else if (tile == 'S') {
                outMap.pickups.emplace_back(pickupCounter++, PickupType::MemorySigil,
                                            Vec3(x, 0.5f, z), "Ada's Crimson Ribbon", "Memory fragment: 'She cried until the doctor locked her in.'");
            } else if (tile == 'C') {
                // Flickering wall candle
                PointLight pl;
                pl.position = Vec3(x, 2.1f, z);
                pl.color = Vec3(1.0f, 0.65f, 0.35f);
                pl.intensity = 1.3f;
                pl.range = 7.5f;
                outMap.pointLights.push_back(pl);

                // Sconce model
                propsBuilder.addCylinder(Vec3(x, 1.8f, z), 0.08f, 0.35f, 8, Vec4(0.3f, 0.3f, 0.3f, 1.0f));
                propsBuilder.addSphere(Vec3(x, 2.18f, z), 0.06f, 6, 8, Vec4(1.0f, 0.8f, 0.2f, 1.0f));
            }
        }
    }

    // Add environmental & architectural props from Props kit
    Props::buildBookshelf(propsBuilder, outMap.playerSpawnPos + Vec3(-4.5f, 0.0f, -3.0f), 90.0f);
    outMap.collision.addBox(AABB(outMap.playerSpawnPos + Vec3(-4.8f, 0.0f, -4.2f), outMap.playerSpawnPos + Vec3(-4.2f, 2.8f, -1.8f)));

    Props::buildBookshelf(propsBuilder, outMap.playerSpawnPos + Vec3(-4.5f, 0.0f, 3.0f), 90.0f);
    outMap.collision.addBox(AABB(outMap.playerSpawnPos + Vec3(-4.8f, 0.0f, 1.8f), outMap.playerSpawnPos + Vec3(-4.2f, 2.8f, 4.2f)));

    Props::buildGrandfatherClock(propsBuilder, outMap.playerSpawnPos + Vec3(3.5f, 0.0f, -4.0f), 0.0f);
    Props::buildLocker(propsBuilder, outMap.playerSpawnPos + Vec3(4.0f, 0.0f, 2.0f), -90.0f);
    Props::buildMirrorPedestal(propsBuilder, outMap.playerSpawnPos + Vec3(-8.0f, 0.0f, 10.0f), 45.0f);
    Props::buildSteamPipeRun(propsBuilder, Vec3(6.0f, 2.8f, -4.0f), Vec3(14.0f, 2.8f, -4.0f));
    Props::buildCandelabra(propsBuilder, outMap.playerSpawnPos + Vec3(2.5f, 0.0f, 5.0f));

    // Wing 5 Chapel High Altar and Pews
    Props::buildAltarTable(propsBuilder, Vec3(0.0f, 0.0f, 22.0f), 0.0f);
    Props::buildPew(propsBuilder, Vec3(-2.0f, 0.0f, 18.0f), 0.0f);
    Props::buildPew(propsBuilder, Vec3(2.0f, 0.0f, 18.0f), 0.0f);
    Props::buildPew(propsBuilder, Vec3(-2.0f, 0.0f, 15.0f), 0.0f);
    Props::buildPew(propsBuilder, Vec3(2.0f, 0.0f, 15.0f), 0.0f);

    // Horror & Visceral Trauma set piece props from PropsGore
    PropsGore::buildSurgicalGurney(propsBuilder, Vec3(8.0f, 0.0f, -8.0f), 30.0f);
    PropsGore::buildSpecimenJar(propsBuilder, Vec3(8.8f, 0.9f, -7.5f));
    PropsGore::buildRestraintChair(propsBuilder, Vec3(11.0f, 0.0f, -10.0f), -45.0f);
    PropsGore::buildHangingHooks(propsBuilder, Vec3(10.0f, 3.4f, -9.0f));
    PropsGore::buildSewnEyeDummy(propsBuilder, Vec3(-10.0f, 0.0f, -8.0f), 15.0f);

    // Upload geometry to GPU
    wallBuilder.build(gl, outMap.wallMesh);
    floorBuilder.build(gl, outMap.floorMesh);
    ceilBuilder.build(gl, outMap.ceilingMesh);
    propsBuilder.build(gl, outMap.propsMesh);

    return true;
}

} // namespace hm
