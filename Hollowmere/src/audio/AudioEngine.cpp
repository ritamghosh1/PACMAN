#include "AudioEngine.h"
#include "scores/motifs.h"
#include <QAudioDeviceInfo>
#include <algorithm>
#include <cmath>

namespace hm {

namespace { constexpr int kGameplayDroneTag = 9001; }

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
        const std::vector<int16_t>& data = v.ref ? *v.ref : v.samples;
        const size_t stride = v.stereo ? 2 : 1;

        for (int i = 0; i < totalSamples; ++i) {
            if (v.cursor + stride > data.size()) {
                if (v.loop && !data.empty()) {
                    v.cursor = 0;
                } else {
                    v.active = false;
                    break;
                }
            }

            if (v.gain != v.gainTarget) {
                v.gain += v.gainStep;
                if ((v.gainStep > 0.0f && v.gain >= v.gainTarget) || (v.gainStep < 0.0f && v.gain <= v.gainTarget)) {
                    v.gain = v.gainTarget;
                    if (v.stopAtSilence && v.gain <= 0.0001f) { v.active = false; break; }
                }
            }
            int16_t sl = data[v.cursor];
            int16_t sr = v.stereo ? data[v.cursor + 1] : sl;
            v.cursor += stride;
            int leftOut = static_cast<int>(out[i * 2 + 0]) + static_cast<int>(sl * v.volumeLeft * v.gain);
            int rightOut = static_cast<int>(out[i * 2 + 1]) + static_cast<int>(sr * v.volumeRight * v.gain);

            out[i * 2 + 0] = static_cast<int16_t>(std::clamp(leftOut, -32767, 32767));
            out[i * 2 + 1] = static_cast<int16_t>(std::clamp(rightOut, -32767, 32767));
        }
    }

    return totalSamples * 2 * sizeof(int16_t);
}

AudioEngine::AudioEngine() {
    m_voices.resize(64); // 64 concurrent voice channels
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

    // Initial ambient drone
    playSound(Synth::generateAmbientDrone(4.0f), 0.0f, 0.40f, true);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& v : m_voices) if (v.active && v.loop) v.tag = kGameplayDroneTag;
    }

    return true;
}

void AudioEngine::playSound(const std::vector<int16_t>& samples, float pan, float volume, bool loop) {
    std::lock_guard<std::mutex> lock(m_mutex);

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
        m_voices[chosenIdx].ref = nullptr;
        m_voices[chosenIdx].stereo = false;
        m_voices[chosenIdx].cursor = 0;
        m_voices[chosenIdx].volumeLeft = volL;
        m_voices[chosenIdx].volumeRight = volR;
        m_voices[chosenIdx].loop = loop;
        m_voices[chosenIdx].tag = 0;
        m_voices[chosenIdx].gain = m_voices[chosenIdx].gainTarget = 1.0f;
        m_voices[chosenIdx].gainStep = 0.0f;
        m_voices[chosenIdx].stopAtSilence = false;
        m_voices[chosenIdx].active = true;
    }
}

void AudioEngine::playSpatial(const std::vector<int16_t>& samples,
                             const Vec3& emitterPos,
                             float volume,
                             float maxDist,
                             bool loop) {
    if (samples.empty()) return;

    // Resolve propagation along portals
    auto prop = SoundPropagation::resolve(m_listener.position, emitterPos, m_portals);

    // Compute spatial binaural cues
    auto spat = Spatial::compute(prop.apparentPosition, m_listener, maxDist, 1.0f, true,
                                !prop.isDirectPath, prop.isBlockedByDoor);

    if (spat.gainLeft < 0.001f && spat.gainRight < 0.001f) return;

    // Filter audio with lowpass if occluded or distant
    std::vector<int16_t> filtered = samples;
    if (spat.lowpassCutoff < 16000.0f) {
        Spatial::applyLowpass(filtered, spat.lowpassCutoff);
    }

    std::lock_guard<std::mutex> lock(m_mutex);

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
        m_voices[chosenIdx].samples = filtered;
        m_voices[chosenIdx].ref = nullptr;
        m_voices[chosenIdx].stereo = false;
        m_voices[chosenIdx].cursor = 0;
        m_voices[chosenIdx].volumeLeft = spat.gainLeft * volume * m_masterVolume;
        m_voices[chosenIdx].volumeRight = spat.gainRight * volume * m_masterVolume;
        m_voices[chosenIdx].loop = loop;
        m_voices[chosenIdx].tag = 0;
        m_voices[chosenIdx].gain = m_voices[chosenIdx].gainTarget = 1.0f;
        m_voices[chosenIdx].gainStep = 0.0f;
        m_voices[chosenIdx].stopAtSilence = false;
        m_voices[chosenIdx].active = true;
    }
}

void AudioEngine::update(float dt, const Vec3& listenerPos, float listenerYawDeg) {
    m_listener.position = listenerPos;
    m_listener.yawDeg = listenerYawDeg;

    m_listenerPos = listenerPos;
    m_musicClock += dt;

    // Once the recordings finish loading, swap the pure-sine procedural drone for a real room drone
    if (!m_assetsAdopted && AssetAudio::instance().ready()) {
        m_assetsAdopted = true;
        if (const AudioClip* drone = AssetAudio::instance().find("room_lowdrone")) {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& v : m_voices)
                if (v.active && v.tag == kGameplayDroneTag) { v.ref = &drone->pcm; v.samples.clear(); v.stereo = false; v.cursor = 0; v.volumeLeft = v.volumeRight = 0.22f * m_masterVolume; }
        }
    }
    if (m_gameplayMuted) return;
    m_musicDirector.update(dt);

    // Procedural score chunks only when there are no recordings to play instead
    if (!m_assetsAdopted) {
        m_musicTimer += dt;
        if (m_musicTimer >= 1.0f) {
            m_musicTimer = 0.0f;
            auto scoreChunk = m_musicDirector.renderChunk(1.0f);
            playSound(scoreChunk, 0.0f, 0.55f, false);
        }
    } else {
        // Random unsettling cues somewhere in the house: a sound you cannot place is scarier than any monster
        m_ambientCueTimer -= dt;
        if (m_ambientCueTimer <= 0.0f) {
            m_ambientCueTimer = 14.0f + 22.0f * ((m_musicClock * 7.31f) - std::floor(m_musicClock * 7.31f));
            const char* zone = m_ctxWing >= 5 ? "amb3_" : (m_ctxWing >= 3 ? "amb2_" : "amb1_");
            if (const AudioClip* cue = AssetAudio::instance().pick(zone)) {
                float a = m_musicClock * 3.7f;
                Vec3 off(std::cos(a) * 9.0f, 0, std::sin(a) * 9.0f);
                playClipSpatial(cue, m_listenerPos + off, 0.55f, 22.0f);
            }
        }
    }

    // Winded: real breathing recordings, louder and more frequent the lower the stamina
    m_breathCooldown -= dt;
    float winded = clamp((0.55f - m_staminaNow) / 0.55f, 0.0f, 1.0f);
    float scared = clamp((m_fearNow - 0.6f) / 0.4f, 0.0f, 1.0f) * 0.5f;
    float need = std::max(winded, scared);
    if (need > 0.05f && m_breathCooldown <= 0.0f) {
        if (const AudioClip* c = AssetAudio::instance().pick("breath_")) {
            playClip(c, 0.30f + 0.65f * need, 0.0f);
            m_breathCooldown = std::max(0.4f, c->seconds * (1.15f - 0.5f * need));
        } else {
            m_breathCooldown = 1.0f;
        }
    }

    // Periodically feed body heartbeat & breath
    m_bodyAudioTimer += dt;
    if (m_bodyAudioTimer >= 0.8f) {
        m_bodyAudioTimer = 0.0f;
        // The body is felt, not announced: heartbeat swells with fear, breathing only when winded
        float heartVol = 0.06f + 0.55f * m_fearNow;
        auto heart = m_bodyAudio.renderHeartbeatChunk(0.8f);
        playSound(heart, 0.0f, heartVol, false);

        if (m_bodyAudio.isTinnitusActive()) {
            auto tin = m_bodyAudio.renderTinnitusChunk(0.8f);
            playSound(tin, 0.0f, 0.40f, false);
        }
    }
}

void AudioEngine::playFootstep(FootstepSurface surface, bool isRightFoot, bool isSprinting, bool isCrouching) {
    // Real recordings when available: shoes on stone for walking, heavier ones for running
    if (const AudioClip* c = AssetAudio::instance().pick(isSprinting ? "run_" : "step_")) {
        float vol = isSprinting ? 0.62f : (isCrouching ? 0.20f : 0.42f);
        playClip(c, vol, isRightFoot ? 0.15f : -0.15f);
        return;
    }
    auto step = m_bodyAudio.generateFootstep(surface, isRightFoot, isSprinting, isCrouching);
    playSound(step, isRightFoot ? 0.15f : -0.15f, 0.42f);
}

void AudioEngine::playDoorCreak(const Vec3& pos) {
    auto& a = AssetAudio::instance();
    if (const AudioClip* c = a.find(m_nextDoorOpens ? "wood_open" : "wood_close")) {
        m_nextDoorOpens = !m_nextDoorOpens;
        playClipSpatial(c, pos, 0.9f, 20.0f);
        return;
    }
    playSpatial(Synth::generateDoorCreak(), pos, 0.85f, 18.0f);
}

void AudioEngine::playDoorSlam(const Vec3& pos) {
    if (const AudioClip* c = AssetAudio::instance().find("wood_close")) {
        playClipSpatial(c, pos, 1.0f, 26.0f);
        if (const AudioClip* g = AssetAudio::instance().find("pd_impact")) playClipSpatial(g, pos, 0.8f, 26.0f);
    } else {
        playSpatial(Synth::generateDoorSlam(), pos, 1.0f, 24.0f);
    }
    m_bodyAudio.triggerTinnitus(2.5f);
}

void AudioEngine::playPickupChime() {
    if (const AudioClip* c = AssetAudio::instance().pick("pick")) { playClip(c, 0.8f); return; }
    playSound(Synth::generatePickupChime(), 0.0f, 0.65f);
}

void AudioEngine::playLanternClick() {
    playSound(Synth::generateLanternClick(), 0.15f, 0.55f);
}

void AudioEngine::playJumpscareStinger() {
    if (const AudioClip* c = AssetAudio::instance().pick("horror_")) playClip(c, 1.0f);
    else playSound(Synth::generateJumpscareStinger(), 0.0f, 1.0f);
    m_bodyAudio.triggerTinnitus(4.0f);
}

void AudioEngine::playWeeperSob(const Vec3& pos) {
    auto& a = AssetAudio::instance();
    if (const AudioClip* c = (m_ghostCue++ % 3 == 0) ? a.find("lure_sniffle") : a.pick("lure_idle")) { playClipSpatial(c, pos, 0.95f, 24.0f); return; }
    playSpatial(Synth2::generateWeeperSobbing(3.0f), pos, 0.90f, 22.0f);
}

void AudioEngine::playHollowManSnip(const Vec3& pos) {
    auto& a = AssetAudio::instance();
    if (const AudioClip* c = (m_ghostCue++ % 4 == 0) ? a.find("pell_breath") : a.pick("pell_rustle")) { playClipSpatial(c, pos, 0.95f, 18.0f); return; }
    playSpatial(Synth2::generateHollowManShearsSnip(), pos, 0.95f, 16.0f);
}

void AudioEngine::playChoirChord(const Vec3& pos, bool dissonant) {
    if (const AudioClip* c = AssetAudio::instance().find(dissonant ? "pd_screech" : "pd_voices")) { playClipSpatial(c, pos, dissonant ? 0.7f : 0.8f, 28.0f); return; }
    playSpatial(Synth2::generateChoirChord(dissonant, 4.0f), pos, 0.90f, 25.0f);
}

void AudioEngine::playDollGiggle(const Vec3& pos) {
    if (const AudioClip* c = AssetAudio::instance().pick("lure_idle")) {
        playClipSpatial(c, pos, 0.85f, 18.0f);
        playSpatial(cinesfx::musicBoxNote(1568.0f, 1.2f), pos, 0.35f, 16.0f);   // the toy-piano plink that marks Eli
        return;
    }
    playSpatial(Synth2::generateDollGiggle(), pos, 0.85f, 18.0f);
}

void AudioEngine::playWardenKeys(const Vec3& pos) {
    auto& a = AssetAudio::instance();
    if (const AudioClip* c = (m_ghostCue++ % 3 == 0) ? a.find("warden_drag") : a.pick("warden_rattle")) {
        playClipSpatial(c, pos, 0.95f, 30.0f);
        playSpatial(Synth2::generateWardenKeyJingle(), pos, 0.45f, 26.0f);
        return;
    }
    playSpatial(Synth2::generateWardenKeyJingle(), pos, 0.95f, 28.0f);
}

void AudioEngine::playWardenRoar(const Vec3& pos) {
    if (const AudioClip* c = AssetAudio::instance().find("pd_rumble")) {
        playClipSpatial(c, pos, 1.0f, 34.0f);
        if (const AudioClip* s = AssetAudio::instance().find("weeper_scream")) playClipSpatial(s, pos, 0.8f, 34.0f);
        m_bodyAudio.triggerTinnitus(3.0f);
        return;
    }
    playSpatial(Synth2::generateWardenGutturalRoar(2.5f), pos, 1.0f, 32.0f);
    m_bodyAudio.triggerTinnitus(3.0f);
}

void AudioEngine::setFearLevel(float fear) {
    m_fearNow = clamp(fear, 0.0f, 1.0f);
    m_bodyAudio.update(0.016f, fear, 1.0f, false, false, false);
    m_musicDirector.setIntensity(fear);
}

void AudioEngine::setStamina(float stamina) {
    m_staminaNow = clamp(stamina, 0.0f, 1.0f);
    m_bodyAudio.update(0.016f, 0.0f, stamina, false, false, false);
}

void AudioEngine::setInjured(bool injured) {
    m_bodyAudio.update(0.016f, 0.0f, 1.0f, injured, false, false);
}

void AudioEngine::setHoldingBreath(bool holding) {
    m_bodyAudio.update(0.016f, 0.0f, 1.0f, false, holding, false);
}

void AudioEngine::setListening(bool listening) {
    m_bodyAudio.update(0.016f, 0.0f, 1.0f, false, false, listening);
}

void AudioEngine::setWingAcoustics(WingAcousticType wing) {
    m_reverb.setWing(wing);
}

void AudioEngine::setMusicIntensity(float intensity) {
    m_musicDirector.setIntensity(intensity);
}

void AudioEngine::setMusicState(MusicState state) {
    m_musicDirector.setState(state);
}

void AudioEngine::triggerSilenceTrap(float durationSec) {
    m_musicDirector.triggerSilenceTrap(durationSec);
}

void AudioEngine::playFlicker() {
    playSound(Synth::generateLanternClick(), 0.0f, 0.7f);
}

void AudioEngine::playWhisper(const Vec3& pos) {
    if (const AudioClip* c = AssetAudio::instance().find("pd_voices")) { playClipSpatial(c, pos, 0.35f, 14.0f); return; }
    // Unvoiced: a breath of wind passing close behind the listener, then a faint tine.
    static const std::vector<int16_t> wind = cinesfx::windBed(2.5f);
    playSpatial(wind, pos, 0.55f, 15.0f);
    playSpatial(cinesfx::musicBoxNote(1318.5f * 0.97f, 1.4f), pos, 0.3f, 12.0f);
}

void AudioEngine::playIntroScore() {
    playSound(scores::renderIntroScore(), 0.0f, 0.85f);
}

void AudioEngine::playEndingScore(int type) {
    playSound(scores::renderEndingScore(type), 0.0f, 0.9f);
}

} // namespace hm

namespace hm {

void AudioEngine::playCine(const std::vector<int16_t>& samples, float volume, float pan) {
    playSound(samples, pan, volume, false);
}

void AudioEngine::startBed(int tag, const std::vector<int16_t>& samples, float volume, float fadeInSec) {
    std::lock_guard<std::mutex> lock(m_mutex);
    AudioVoice* slot = nullptr;
    for (auto& v : m_voices) if (v.active && v.tag == tag) { slot = &v; break; }   // restart in place
    if (!slot) for (auto& v : m_voices) if (!v.active) { slot = &v; break; }
    if (!slot) return;
    slot->samples = samples;
    slot->ref = nullptr;
    slot->stereo = false;
    slot->cursor = 0;
    slot->loop = true;
    slot->tag = tag;
    slot->volumeLeft = slot->volumeRight = volume * m_masterVolume;
    slot->stopAtSilence = false;
    slot->gain = 0.0f;
    slot->gainTarget = 1.0f;
    slot->gainStep = 1.0f / std::max(1.0f, fadeInSec * static_cast<float>(Synth::SAMPLE_RATE));
    slot->active = true;
}

void AudioEngine::stopBed(int tag, float fadeOutSec) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& v : m_voices) {
        if (!v.active || v.tag != tag) continue;
        v.gainTarget = 0.0f;
        v.gainStep = -std::max(0.0001f, v.gain) / std::max(1.0f, fadeOutSec * static_cast<float>(Synth::SAMPLE_RATE));
        v.stopAtSilence = true;
    }
}

void AudioEngine::setBedVolume(int tag, float volume, float rampSec) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& v : m_voices) {
        if (!v.active || v.tag != tag) continue;
        float base = std::max(0.0001f, volume * m_masterVolume);
        float ratio = base / std::max(0.0001f, std::max(v.volumeLeft, v.volumeRight));
        v.gainTarget = ratio;
        v.gainStep = (ratio - v.gain) / std::max(1.0f, rampSec * static_cast<float>(Synth::SAMPLE_RATE));
        v.stopAtSilence = false;
    }
}

void AudioEngine::stopAllBeds(float fadeOutSec) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& v : m_voices) {
        if (!v.active || v.tag == 0 || v.tag == kGameplayDroneTag) continue;
        v.gainTarget = 0.0f;
        v.gainStep = -std::max(0.0001f, v.gain) / std::max(1.0f, fadeOutSec * static_cast<float>(Synth::SAMPLE_RATE));
        v.stopAtSilence = true;
    }
}

void AudioEngine::playClip(const AudioClip* clip, float volume, float pan) {
    if (!clip || clip->pcm.empty()) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    AudioVoice* slot = nullptr;
    for (auto& v : m_voices) if (!v.active) { slot = &v; break; }
    if (!slot) for (auto& v : m_voices) if (!v.loop) { slot = &v; break; }
    if (!slot) return;
    pan = clamp(pan, -1.0f, 1.0f);
    slot->ref = &clip->pcm;
    slot->samples.clear();
    slot->stereo = clip->stereo;
    slot->cursor = 0;
    slot->volumeLeft = volume * std::min(1.0f, 1.0f - pan) * m_masterVolume;
    slot->volumeRight = volume * std::min(1.0f, 1.0f + pan) * m_masterVolume;
    slot->loop = false;
    slot->tag = 0;
    slot->gain = slot->gainTarget = 1.0f;
    slot->gainStep = 0.0f;
    slot->stopAtSilence = false;
    slot->active = true;
}

void AudioEngine::playClipSpatial(const AudioClip* clip, const Vec3& pos, float volume, float maxDist) {
    if (!clip || clip->pcm.empty() || clip->stereo) return;
    auto prop = SoundPropagation::resolve(m_listener.position, pos, m_portals);
    auto spat = Spatial::compute(prop.apparentPosition, m_listener, maxDist, 1.0f, true, !prop.isDirectPath, prop.isBlockedByDoor);
    if (spat.gainLeft < 0.001f && spat.gainRight < 0.001f) return;
    std::vector<int16_t> filtered = clip->pcm;      // lowpass copy: walls and distance muffle the recording
    if (spat.lowpassCutoff < 16000.0f) Spatial::applyLowpass(filtered, spat.lowpassCutoff);
    std::lock_guard<std::mutex> lock(m_mutex);
    AudioVoice* slot = nullptr;
    for (auto& v : m_voices) if (!v.active) { slot = &v; break; }
    if (!slot) for (auto& v : m_voices) if (!v.loop) { slot = &v; break; }
    if (!slot) return;
    slot->samples = std::move(filtered);
    slot->ref = nullptr;
    slot->stereo = false;
    slot->cursor = 0;
    slot->volumeLeft = spat.gainLeft * volume * m_masterVolume;
    slot->volumeRight = spat.gainRight * volume * m_masterVolume;
    slot->loop = false;
    slot->tag = 0;
    slot->gain = slot->gainTarget = 1.0f;
    slot->gainStep = 0.0f;
    slot->stopAtSilence = false;
    slot->active = true;
}

void AudioEngine::startClipBed(int tag, const AudioClip* clip, float volume, float fadeInSec) {
    if (!clip || clip->pcm.empty()) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    AudioVoice* slot = nullptr;
    for (auto& v : m_voices) if (v.active && v.tag == tag) { slot = &v; break; }
    if (!slot) for (auto& v : m_voices) if (!v.active) { slot = &v; break; }
    if (!slot) return;
    slot->ref = &clip->pcm;
    slot->samples.clear();
    slot->stereo = clip->stereo;
    slot->cursor = 0;
    slot->loop = true;
    slot->tag = tag;
    slot->volumeLeft = slot->volumeRight = volume * m_masterVolume;
    slot->stopAtSilence = false;
    slot->gain = 0.0f;
    slot->gainTarget = 1.0f;
    slot->gainStep = 1.0f / std::max(1.0f, fadeInSec * static_cast<float>(Synth::SAMPLE_RATE));
    slot->active = true;
}

void AudioEngine::setExplorationContext(int wing, bool inSafe, int chaseGhost) {
    m_ctxWing = wing;
    if (m_gameplayMuted || !AssetAudio::instance().ready()) return;
    static const char* wingTracks[6] = {"music_foyer", "music_library", "music_garden", "music_cellar", "music_nursery", "music_chapel"};
    static const char* chaseTracks[5] = {"music_library_chase", "music_chase_pell", "music_cellar", "music_chase_doll", "music_chase_warden"};
    std::string want = chaseGhost >= 0 ? chaseTracks[std::clamp(chaseGhost, 0, 4)] : wingTracks[std::clamp(wing, 0, 5)];
    float vol = chaseGhost >= 0 ? 0.85f : (inSafe ? 0.30f : 0.55f);
    bool chasing = chaseGhost >= 0;
    if (want != m_musicName) {
        // Changing track costs a crossfade; don't thrash when a ghost flickers in and out of range
        bool urgent = chasing || m_musicName.empty();
        if (!urgent && m_musicClock - m_musicSwitchedAt < 8.0f) return;
        if (const AudioClip* clip = AssetAudio::instance().find(want)) {
            int old = m_musicTag;
            m_musicTag = (m_musicTag == 200) ? 201 : 200;
            stopBed(old, 3.0f);
            startClipBed(m_musicTag, clip, vol, urgent ? 1.5f : 4.0f);
            m_musicName = want;
            m_musicSwitchedAt = m_musicClock;
            m_musicVolume = vol;
        }
    } else if (std::fabs(vol - m_musicVolume) > 0.02f) {
        setBedVolume(m_musicTag, vol, 2.0f);   // duck in safe rooms, swell when something hunts you
        m_musicVolume = vol;
    }
}

void AudioEngine::playTypeTick(Speaker who) {
    if (m_tickCache.empty()) {
        for (int s = 0; s < static_cast<int>(Speaker::Count); ++s)
            for (int v = 0; v < 4; ++v) m_tickCache.push_back(cinesfx::typeTick(static_cast<Speaker>(s), v));
    }
    ++m_tickCounter;
    int idx = static_cast<int>(who) * 4 + ((m_tickCounter * 3 + (m_tickCounter >> 2)) & 3);
    playSound(m_tickCache[idx], 0.0f, 0.55f, false);
}

void AudioEngine::playSpeakerMotif(Speaker who) {
    playSound(cinesfx::motif(who), 0.0f, 0.75f, false);
}

void AudioEngine::playUi(int kind) {
    static const std::vector<int16_t> hover = cinesfx::uiHover(), click = cinesfx::uiClick(), adv = cinesfx::uiAdvance();
    playSound(kind == 0 ? hover : (kind == 1 ? click : adv), 0.0f, 0.6f, false);
}

void AudioEngine::setGameplayMuted(bool muted) {
    m_gameplayMuted = muted;
    if (muted) { stopBed(200, 1.2f); stopBed(201, 1.2f); }
    m_musicName.clear();   // re-pick the right track when gameplay resumes
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& v : m_voices) {
        if (!v.active || v.tag != kGameplayDroneTag) continue;
        v.gainTarget = muted ? 0.0f : 1.0f;
        v.gainStep = (v.gainTarget - v.gain) / (1.5f * Synth::SAMPLE_RATE);
    }
}

} // namespace hm
