#include "Clock.h"
#include <algorithm>

namespace hm {

Clock::Clock() {
    reset();
}

void Clock::reset() {
    m_lastTime = std::chrono::high_resolution_clock::now();
    m_accumulator = 0.0;
    m_alpha = 0.0;
    m_frameTime = 0.0;
    m_fps = 60.0;
    m_fpsTimer = 0.0;
    m_frameCount = 0;
}

void Clock::tick() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - m_lastTime;
    m_lastTime = now;

    m_frameTime = std::min(elapsed.count(), config::MAX_FRAME_TIME);
    m_accumulator += m_frameTime;

    // FPS calculation
    m_fpsTimer += m_frameTime;
    m_frameCount++;
    if (m_fpsTimer >= 0.5) {
        m_fps = m_frameCount / m_fpsTimer;
        m_fpsTimer = 0.0;
        m_frameCount = 0;
    }
}

bool Clock::hasStep() {
    return m_accumulator >= config::SIM_DT;
}

void Clock::consumeStep() {
    m_accumulator -= config::SIM_DT;
    m_alpha = m_accumulator / config::SIM_DT;
}

} // namespace hm
