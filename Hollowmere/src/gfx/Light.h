#pragma once

#include "core/mathx.h"

namespace hm {

struct SpotLight {
    Vec3 position{0, 1.5f, 0};
    Vec3 direction{0, 0, -1};
    Vec3 color{1.0f, 0.85f, 0.65f}; // warm lantern flame
    float intensity = 1.6f;
    float innerCutoff = 0.92f;
    float outerCutoff = 0.82f;
    float range = 18.0f;
    bool enabled = true;
};

struct PointLight {
    Vec3 position{0, 2.0f, 0};
    Vec3 color{1.0f, 0.7f, 0.4f};
    float intensity = 1.0f;
    float range = 8.0f;
    bool enabled = true;
};

struct AmbientLight {
    Vec3 color{0.035f, 0.045f, 0.065f}; // cold night gloom
    float intensity = 1.0f;
};

} // namespace hm
