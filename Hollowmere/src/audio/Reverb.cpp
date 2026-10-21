#include "Reverb.h"

namespace hm {

ReverbManager::ReverbManager() {
    setWing(WingAcousticType::StoneHall);
}

void ReverbManager::setWing(WingAcousticType type) {
    m_type = type;
    FdnReverb::RoomAcoustics room;

    switch (type) {
        case WingAcousticType::StoneHall:
            room.rt60 = 2.2f;
            room.damping = 0.35f;
            room.wetLevel = 0.30f;
            room.roomScale = 1.2f;
            m_earlyRefl.configureRoom(12.0f, 6.0f, 16.0f, 0.25f);
            break;
        case WingAcousticType::WoodLibrary:
            room.rt60 = 1.1f;
            room.damping = 0.55f;
            room.wetLevel = 0.22f;
            room.roomScale = 0.9f;
            m_earlyRefl.configureRoom(8.0f, 3.8f, 10.0f, 0.45f);
            break;
        case WingAcousticType::GlassDome:
            room.rt60 = 1.7f;
            room.damping = 0.20f;
            room.wetLevel = 0.35f;
            room.roomScale = 1.1f;
            m_earlyRefl.configureRoom(10.0f, 5.0f, 12.0f, 0.15f);
            break;
        case WingAcousticType::DampCellar:
            room.rt60 = 1.9f;
            room.damping = 0.28f;
            room.wetLevel = 0.32f;
            room.roomScale = 1.0f;
            m_earlyRefl.configureRoom(9.0f, 3.0f, 14.0f, 0.20f);
            break;
        case WingAcousticType::SoftNursery:
            room.rt60 = 0.75f;
            room.damping = 0.70f;
            room.wetLevel = 0.16f;
            room.roomScale = 0.7f;
            m_earlyRefl.configureRoom(6.0f, 2.8f, 6.0f, 0.65f);
            break;
        case WingAcousticType::Cathedral:
            room.rt60 = 3.6f;
            room.damping = 0.30f;
            room.wetLevel = 0.42f;
            room.roomScale = 1.8f;
            m_earlyRefl.configureRoom(16.0f, 10.0f, 24.0f, 0.18f);
            break;
    }

    m_fdn.setRoom(room);
}

void ReverbManager::processStereo(std::vector<int16_t>& stereoSamples, float wetFactor) {
    if (stereoSamples.empty()) return;

    size_t numStereo = stereoSamples.size() / 2;
    for (size_t i = 0; i < numStereo; ++i) {
        float inL = static_cast<float>(stereoSamples[i * 2 + 0]);
        float inR = static_cast<float>(stereoSamples[i * 2 + 1]);
        float monoIn = (inL + inR) * 0.5f;

        float outL = inL;
        float outR = inR;
        m_fdn.processSample(monoIn, outL, outR);

        float blendedL = inL + (outL - inL) * wetFactor;
        float blendedR = inR + (outR - inR) * wetFactor;

        stereoSamples[i * 2 + 0] = static_cast<int16_t>(clamp(blendedL, -32767.0f, 32767.0f));
        stereoSamples[i * 2 + 1] = static_cast<int16_t>(clamp(blendedR, -32767.0f, 32767.0f));
    }
}

} // namespace hm
