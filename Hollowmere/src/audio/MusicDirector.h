#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace hm {

enum class MusicState {
    Silence,
    DreadBed,
    Pursuit,
    Hiding,
    Emotional,
    SafeRoom
};

class MusicDirector {
public:
    MusicDirector();

    void update(float dt);

    void setIntensity(float intensity); // 0.0 (peaceful/dread) to 1.0 (deadly pursuit)
    float intensity() const { return m_intensity; }

    void setState(MusicState state);
    MusicState state() const { return m_state; }

    // Silence trap: sudden cut to dead silence for dramatic scare preparation
    void triggerSilenceTrap(float durationSec = 3.0f);
    bool isSilenceTrapActive() const { return m_silenceTrapTimer > 0.0f; }

    // Stem volume levels (0.0 to 1.0)
    float stemLevelL0() const { return m_stemL0; }
    float stemLevelL1() const { return m_stemL1; }
    float stemLevelL2() const { return m_stemL2; }
    float stemLevelL3() const { return m_stemL3; }

    // Generates a chunk of mixed score audio according to current stems & state
    std::vector<int16_t> renderChunk(float durationSec = 1.0f);

private:
    MusicState m_state = MusicState::DreadBed;
    float m_intensity = 0.0f;
    float m_targetIntensity = 0.0f;
    float m_silenceTrapTimer = 0.0f;

    float m_stemL0 = 1.0f; // Ambient drone
    float m_stemL1 = 0.0f; // Cello pad
    float m_stemL2 = 0.0f; // Heartbeat pulse / tension
    float m_stemL3 = 0.0f; // Pursuit stingers / riser

    float m_scorePhase = 0.0f;
};

} // namespace hm
