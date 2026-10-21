#pragma once

#include <QIODevice>
#include <QAudioOutput>
#include <vector>
#include <mutex>
#include "Synth.h"
#include "core/mathx.h"

namespace hm {

struct AudioVoice {
    std::vector<int16_t> samples;
    size_t cursor = 0;
    float volumeLeft = 1.0f;
    float volumeRight = 1.0f;
    bool loop = false;
    bool active = false;
};

class AudioStreamDevice : public QIODevice {
    Q_OBJECT
public:
    AudioStreamDevice(std::vector<AudioVoice>& voices, std::mutex& mutex);

    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char*, qint64) override { return 0; }
    qint64 bytesAvailable() const override { return 4096; }

private:
    std::vector<AudioVoice>& m_voices;
    std::mutex& m_mutex;
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool init();
    void update(const Vec3& listenerPos, float listenerYawDeg);

    void playFootstep();
    void playDoorCreak(const Vec3& pos);
    void playDoorSlam(const Vec3& pos);
    void playPickupChime();
    void playLanternClick();
    void playJumpscareStinger();
    void setHeartbeatBpm(float bpm);
    void setGhostWailDistance(float dist, float panAngle);

    void setMasterVolume(float vol) { m_masterVolume = vol; }

private:
    void playSound(const std::vector<int16_t>& samples, float pan = 0.0f, float volume = 1.0f, bool loop = false);

    QAudioOutput* m_audioOutput = nullptr;
    AudioStreamDevice* m_streamDevice = nullptr;

    std::vector<AudioVoice> m_voices;
    std::mutex m_mutex;

    float m_masterVolume = 1.0f;
    Vec3 m_listenerPos{0, 0, 0};
    float m_listenerYaw = 0.0f;
};

} // namespace hm
