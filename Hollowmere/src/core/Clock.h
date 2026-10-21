#pragma once

#include <chrono>
#include "config.h"

namespace hm {

class Clock {
public:
    Clock();

    void reset();
    void tick();

    bool hasStep();
    void consumeStep();

    double alpha() const { return m_alpha; }
    double deltaSim() const { return config::SIM_DT; }
    double frameTime() const { return m_frameTime; }
    double fps() const { return m_fps; }

private:
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    TimePoint m_lastTime;
    double m_accumulator = 0.0;
    double m_alpha = 0.0;
    double m_frameTime = 0.0;

    double m_fps = 60.0;
    double m_fpsTimer = 0.0;
    int m_frameCount = 0;
};

} // namespace hm
