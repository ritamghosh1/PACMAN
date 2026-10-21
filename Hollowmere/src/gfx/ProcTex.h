#pragma once

#include <QImage>
#include "Texture.h"
#include "core/rng.h"
#include "core/mathx.h"

namespace hm {

class ProcTex {
public:
    static QImage generateStoneBrick(int size = 512, uint32_t seed = 42);
    static QImage generateWoodPlanks(int size = 512, uint32_t seed = 101);
    static QImage generatePeelingPlaster(int size = 512, uint32_t seed = 202);
    static QImage generateCheckerTile(int size = 512, uint32_t seed = 303);
    static QImage generateRustMetal(int size = 512, uint32_t seed = 404);
    static QImage generateBloodSplatter(int size = 512, uint32_t seed = 505);

    // Converts a grayscale heightmap image into a tangent-space normal map
    static QImage generateNormalMap(const QImage& heightMap, float strength = 2.5f);

private:
    static float perlinNoise(float x, float y);
    static float fbm(float x, float y, int octaves = 4);
    static float worley(float x, float y);
};

} // namespace hm
