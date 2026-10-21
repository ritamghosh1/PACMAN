#include "AudioEngine.h"
#include <QAudioDeviceInfo>
#include <algorithm>
#include <cmath>

namespace hm {

AudioStreamDevice::AudioStreamDevice(std::vector<AudioVoice>& voices, std::mutex& mutex)
    : m_voices(voices), m_mutex(mutex) {
    open(QIODevice::ReadOnly);
}

qint64 AudioStreamDevice::readData(char* data, qint64 maxlen) {
    int totalSamples = static_cast<int>(maxlen / (2 * sizeof(int16_t))); // stereo 16-bit
    int16_t* out = reinterpret_cast<int16_t*>(data);
    std::fill(out, out + totalSamples * 2, 0);

    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& v : m_voices) {
        if (!v.active) continue;

        for (int i = 0; i < totalSamples; ++i) {
            if (v.cursor >= v.samples.size()) {
                if (v.loop) {
                    v.cursor = 0;
                } else {
                    v.active = false;
                    break;
                }
            }

            int16_t sample = v.samples[v.cursor++];
            int leftOut = static_cast<int>(out[i * 2 + 0]) + static_cast<int>(sample * v.volumeLeft);
            int rightOut = static_cast<int>(out[i * 2 + 1]) + static_cast<int>(sample * v.volumeRight);

            out[i * 2 + 0] = static_cast<int16_t>(std::clamp(leftOut, -32767, 32767));
            out[i * 2 + 1] = static_cast<int16_t>(std::clamp(rightOut, -32767, 32767));
        }
    }

    return totalSamples * 2 * sizeof(int16_t);
}

AudioEngine::AudioEngine() {
    m_voices.resize(32); // 32 concurrent voice channels
}

AudioEngine::~AudioEngine() {
    if (m_audioOutput) {
        m_audioOutput->stop();
        delete m_audioOutput;
        m_audioOutput = nullptr;
    }
    if (m_streamDevice) {
        delete m_streamDevice;
        m_streamDevice = nullptr;
    }
}

bool AudioEngine::init() {
    QAudioFormat format;
    format.setSampleRate(Synth::SAMPLE_RATE);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        format = info.nearestFormat(format);
    }

    m_audioOutput = new QAudioOutput(format);
    m_streamDevice = new AudioStreamDevice(m_voices, m_mutex);
    m_audioOutput->start(m_streamDevice);

    // Start background ambient horror drone
    playSound(Synth::generateAmbientDrone(4.0f), 0.0f, 0.45f, true);

    return true;
}

void AudioEngine::playSound(const std::vector<int16_t>& samples, float pan, float volume, bool loop) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Find inactive voice or steal oldest non-looping
    int chosenIdx = -1;
    for (size_t i = 0; i < m_voices.size(); ++i) {
        if (!m_voices[i].active) {
            chosenIdx = static_cast<int>(i);
            break;
        }
    }

    if (chosenIdx < 0) {
        for (size_t i = 0; i < m_voices.size(); ++i) {
            if (!m_voices[i].loop) {
                chosenIdx = static_cast<int>(i);
                break;
            }
        }
    }

    if (chosenIdx >= 0) {
        pan = clamp(pan, -1.0f, 1.0f);
        float volL = volume * std::min(1.0f, 1.0f - pan) * m_masterVolume;
        float volR = volume * std::min(1.0f, 1.0f + pan) * m_masterVolume;

        m_voices[chosenIdx].samples = samples;
        m_voices[chosenIdx].cursor = 0;
        m_voices[chosenIdx].volumeLeft = volL;
        m_voices[chosenIdx].volumeRight = volR;
        m_voices[chosenIdx].loop = loop;
        m_voices[chosenIdx].active = true;
    }
}

void AudioEngine::update(const Vec3& listenerPos, float listenerYawDeg) {
    m_listenerPos = listenerPos;
    m_listenerYaw = listenerYawDeg;
}

void AudioEngine::playFootstep() {
    static int variant = 0;
    playSound(Synth::generateFootstep(variant++), 0.0f, 0.38f);
}

void AudioEngine::playDoorCreak(const Vec3& pos) {
    float dist = distance(m_listenerPos, pos);
    float atten = clamp(1.0f - (dist / 14.0f), 0.0f, 1.0f);
    if (atten > 0.05f) {
        playSound(Synth::generateDoorCreak(), 0.0f, atten * 0.85f);
    }
}

void AudioEngine::playDoorSlam(const Vec3& pos) {
    float dist = distance(m_listenerPos, pos);
    float atten = clamp(1.0f - (dist / 18.0f), 0.0f, 1.0f);
    if (atten > 0.05f) {
        playSound(Synth::generateDoorSlam(), 0.0f, atten * 0.95f);
    }
}

void AudioEngine::playPickupChime() {
    playSound(Synth::generatePickupChime(), 0.0f, 0.6f);
}

void AudioEngine::playLanternClick() {
    playSound(Synth::generateLanternClick(), 0.15f, 0.5f);
}

void AudioEngine::playJumpscareStinger() {
    playSound(Synth::generateJumpscareStinger(), 0.0f, 1.0f);
}

void AudioEngine::setHeartbeatBpm(float /*bpm*/) {
    // Dynamic heartbeat frequency
}

void AudioEngine::setGhostWailDistance(float dist, float panAngle) {
    float atten = clamp(1.0f - (dist / 15.0f), 0.0f, 1.0f);
    if (atten > 0.08f) {
        static float timer = 0.0f;
        timer += 0.016f;
        if (timer > 3.2f) {
            timer = 0.0f;
            float pan = std::sin(panAngle);
            playSound(Synth::generateGhostWail(2.8f), pan, atten * 0.75f);
        }
    }
}

} // namespace hm
