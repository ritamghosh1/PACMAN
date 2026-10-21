#include "Spatial.h"
#include <cmath>
#include <algorithm>

namespace hm {

SpatialParams Spatial::compute(const Vec3& emitterPos,
                               const Listener3D& listener,
                               float maxDistance,
                               float rolloff,
                               bool binauralMode,
                               bool directOcclusion,
                               bool closedDoor) {
    SpatialParams p;
    Vec3 diff = emitterPos - listener.position;
    p.distance = length(diff);

    // Distance attenuation with soft rolloff
    if (p.distance < 0.001f) {
        p.gainLeft = 1.0f;
        p.gainRight = 1.0f;
        p.lowpassCutoff = 20000.0f;
        return p;
    }

    float distFactor = 1.0f / (1.0f + rolloff * 0.2f * p.distance);
    float rangeFade = clamp(1.0f - (p.distance / maxDistance), 0.0f, 1.0f);
    float overallGain = distFactor * rangeFade;

    // Atmospheric air absorption
    p.lowpassCutoff = 20000.0f / (1.0f + p.distance * 0.04f);

    // Occlusion penalties
    if (closedDoor) {
        overallGain *= 0.125f; // ~ -18 dB
        p.lowpassCutoff = std::min(p.lowpassCutoff, 900.0f);
        p.isOccluded = true;
    } else if (directOcclusion) {
        overallGain *= 0.35f;  // ~ -9 dB
        p.lowpassCutoff = std::min(p.lowpassCutoff, 1800.0f);
        p.isOccluded = true;
    }

    // Compute relative azimuth and elevation in listener coordinate frame
    float radYaw = -radians(listener.yawDeg);
    // Rotate diff around Y axis into listener space
    float localX = diff.x * std::cos(radYaw) - diff.z * std::sin(radYaw);
    float localZ = diff.x * std::sin(radYaw) + diff.z * std::cos(radYaw);
    float localY = diff.y;

    // Azimuth: -180 to +180 deg. 0 = front (-Z), +90 = right (+X), -90 = left (-X)
    p.azimuthDeg = degrees(std::atan2(localX, -localZ));
    float horizDist = std::sqrt(localX * localX + localZ * localZ);
    p.elevationDeg = degrees(std::atan2(localY, std::max(0.001f, horizDist)));

    // Equal power panning
    float panNorm = clamp(p.azimuthDeg / 90.0f, -1.0f, 1.0f); // -1 left, +1 right
    float panAngle = (panNorm + 1.0f) * 0.25f * PI; // 0 to pi/2
    float panL = std::cos(panAngle);
    float panR = std::sin(panAngle);

    if (!binauralMode) {
        p.gainLeft = overallGain * panL;
        p.gainRight = overallGain * panR;
        p.delaySamplesLeft = 0;
        p.delaySamplesRight = 0;
        return p;
    }

    // Binaural HRTF-lite:
    // ITD Woodworth formula: Delay = (r/c) * (sin(theta) + theta)
    float theta = std::abs(radians(p.azimuthDeg));
    if (theta > PI * 0.5f) {
        theta = PI - theta;
    }
    float delaySec = MAX_ITD_SEC * (std::sin(theta) + theta * 0.4f);
    int delaySamples = static_cast<int>(delaySec * SAMPLE_RATE);

    // Pinna notch simulation for back-vs-front disambiguation
    bool isBehind = (std::abs(p.azimuthDeg) > 90.0f);
    if (isBehind) {
        // Pinna shadowing reduces 4kHz - 8kHz slightly and drops gain
        overallGain *= 0.88f;
        p.lowpassCutoff = std::min(p.lowpassCutoff, 8500.0f);
    }

    // Far-ear head shadow attenuation (ILD)
    float ildShadow = 0.55f; // high-frequency head shadow
    if (p.azimuthDeg > 0.0f) {
        // Sound is to the right
        p.gainLeft = overallGain * panL * ildShadow;
        p.gainRight = overallGain * panR;
        p.delaySamplesLeft = delaySamples;
        p.delaySamplesRight = 0;
    } else {
        // Sound is to the left
        p.gainLeft = overallGain * panL;
        p.gainRight = overallGain * panR * ildShadow;
        p.delaySamplesLeft = 0;
        p.delaySamplesRight = delaySamples;
    }

    return p;
}

void Spatial::applyLowpass(std::vector<int16_t>& samples, float cutoffHz, int sampleRate) {
    if (cutoffHz >= 18000.0f || samples.empty()) return;

    // 1-pole IIR low-pass filter
    float dt = 1.0f / static_cast<float>(sampleRate);
    float rc = 1.0f / (2.0f * PI * cutoffHz);
    float alpha = dt / (rc + dt);

    float prev = 0.0f;
    for (size_t i = 0; i < samples.size(); ++i) {
        float in = static_cast<float>(samples[i]);
        prev = prev + alpha * (in - prev);
        samples[i] = static_cast<int16_t>(clamp(prev, -32767.0f, 32767.0f));
    }
}

} // namespace hm
