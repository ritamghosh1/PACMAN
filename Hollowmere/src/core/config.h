#pragma once

#include <cstdint>
#include <cmath>
#include <QDebug>

namespace hm {

namespace config {
    constexpr const char* GAME_NAME = "Hollowmere";
    constexpr const char* VERSION_STR = "1.0.0-alpha";

    constexpr int DEFAULT_WINDOW_WIDTH = 1280;
    constexpr int DEFAULT_WINDOW_HEIGHT = 720;

    constexpr double SIM_DT = 1.0 / 120.0; // 120 Hz simulation rate
    constexpr double MAX_FRAME_TIME = 0.25;

    // Player tunings
    namespace tune {
        constexpr float WALK_SPEED = 2.0f;
        constexpr float SPRINT_SPEED = 3.6f;
        constexpr float CROUCH_SPEED = 1.0f;
        constexpr float PLAYER_RADIUS = 0.32f;
        constexpr float STAND_EYE_HEIGHT = 1.65f;
        constexpr float CROUCH_EYE_HEIGHT = 0.95f;
        constexpr float INTERACT_DISTANCE = 2.2f;

        constexpr float STAMINA_MAX = 6.0f;
        constexpr float STAMINA_RECOVERY_RATE = 1.2f;
        constexpr float STAMINA_DRAIN_RATE = 1.0f;

        constexpr float LANTERN_MAX_OIL = 100.0f;
        constexpr float LANTERN_OIL_DRAIN_NORMAL = 100.0f / 300.0f; // 5 mins full tank
        constexpr float LANTERN_OIL_DRAIN_FOCUSED = 100.0f / 100.0f; // 1.6 mins when focused
        constexpr float LANTERN_BASE_INNER_CUTOFF = 0.92f;  // ~23 deg
        constexpr float LANTERN_BASE_OUTER_CUTOFF = 0.82f;  // ~35 deg
        constexpr float LANTERN_FOCUS_INNER_CUTOFF = 0.97f; // ~14 deg
        constexpr float LANTERN_FOCUS_OUTER_CUTOFF = 0.93f; // ~21 deg
    }
}

} // namespace hm

#ifdef QT_DEBUG
#define HM_GL_CHECK(gl) \
    do { \
        GLenum err; \
        while ((err = gl->glGetError()) != GL_NO_ERROR) { \
            qWarning("OpenGL error 0x%04x at %s:%d", err, __FILE__, __LINE__); \
        } \
    } while (0)
#else
#define HM_GL_CHECK(gl) do {} while (0)
#endif
