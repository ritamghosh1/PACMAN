#pragma once

#include <QIODevice>
#include <QAudioOutput>
#include <vector>
#include <mutex>
#include <string>
#include "Synth.h"
#include "Synth2.h"
#include "Spatial.h"
#include "Reverb.h"
#include "Propagation.h"
#include "MusicDirector.h"
#include "BodyAudio.h"
#include "CineSfx.h"
#include "AssetAudio.h"
#include "core/mathx.h"

namespace hm {

struct AudioVoice {
    std::vector<int16_t> samples;
    const std::vector<int16_t>* ref = nullptr;   // points at a long-lived AudioClip instead of copying it
    bool stereo = false;                          // interleaved L/R (music)
    size_t cursor = 0;
    float volumeLeft = 1.0f;
    float volumeRight = 1.0f;
    bool loop = false;
    bool active = false;
    int tag = 0;               // non-zero: addressable by stopTag() (ambience beds, scores)
    float gain = 1.0f;         // current ramped gain
    float gainTarget = 1.0f;
    float gainStep = 0.0f;     // per-sample ramp; lets beds fade instead of clicking off
    bool stopAtSilence = false;
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
    void update(float dt, const Vec3& listenerPos, float listenerYawDeg);
    void update(const Vec3& listenerPos, float listenerYawDeg, float dt = 0.016f) {
        update(dt, listenerPos, listenerYawDeg);
    }

    // 3D Spatial Audio playback
    void playSpatial(const std::vector<int16_t>& samples,
                     const Vec3& emitterPos,
                     float volume = 1.0f,
                     float maxDist = 25.0f,
                     bool loop = false);

    // Standard Foley & interactions
    void playFootstep(FootstepSurface surface = FootstepSurface::Wood,
                      bool isRightFoot = false,
                      bool isSprinting = false,
                      bool isCrouching = false);
    void playDoorCreak(const Vec3& pos);
    void playDoorSlam(const Vec3& pos);
    void playPickupChime();
    void playLanternClick();
    void playJumpscareStinger();
    void setHeartbeatBpm(float bpm) { (void)bpm; }
    void setGhostWailDistance(float dist, float panAngle) { (void)dist; (void)panAngle; }

    // Ghost acoustic signatures
    void playWeeperSob(const Vec3& pos);
    void playHollowManSnip(const Vec3& pos);
    void playChoirChord(const Vec3& pos, bool dissonant);
    void playDollGiggle(const Vec3& pos);
    void playWardenKeys(const Vec3& pos);
    void playWardenRoar(const Vec3& pos);
    void playFlicker();
    void playWhisper(const Vec3& pos);

    // Musical scores & cutscenes
    void playIntroScore();
    void playEndingScore(int type);

    // Story presentation (no voice acting: ambience, motifs and typewriter ticks carry the scene)
    void playCine(const std::vector<int16_t>& samples, float volume = 1.0f, float pan = 0.0f);
    void startBed(int tag, const std::vector<int16_t>& samples, float volume, float fadeInSec = 1.5f);
    void stopBed(int tag, float fadeOutSec = 1.5f);
    void setBedVolume(int tag, float volume, float rampSec = 0.5f);
    void stopAllBeds(float fadeOutSec = 1.0f);
    // Recordings (assets/audio). All of these fall back silently if the files are not present.
    bool haveAssets() const { return AssetAudio::instance().ready(); }
    void playClip(const AudioClip* clip, float volume = 1.0f, float pan = 0.0f);
    void playClipSpatial(const AudioClip* clip, const Vec3& pos, float volume = 1.0f, float maxDist = 25.0f);
    void startClipBed(int tag, const AudioClip* clip, float volume, float fadeInSec = 2.0f);   // looping, any tag
    // Gameplay music director: chooses the track from where the player is and what is hunting them.
    // chaseGhost: -1 none, otherwise the GhostType index (Weeper 0 .. Warden 4)
    void stopGameplayMusic() { stopBed(200, 1.5f); stopBed(201, 1.5f); m_musicName.clear(); }
    void setExplorationContext(int wing, bool inSafeRoom, int chaseGhost);
    void playTypeTick(Speaker who);
    void playSpeakerMotif(Speaker who);
    void playUi(int kind); // 0 hover, 1 click, 2 advance
    void setGameplayMuted(bool muted);  // silences the ambient drone/score/body layers during cutscenes

    // Player body state
    void setFearLevel(float fear);
    void setStamina(float stamina);
    void setInjured(bool injured);
    void setHoldingBreath(bool holding);
    void setListening(bool listening);

    // Room acoustics & music director
    void setWingAcoustics(WingAcousticType wing);
    void setMusicIntensity(float intensity);
    void setMusicState(MusicState state);
    void triggerSilenceTrap(float durationSec = 3.0f);
    void setMasterVolume(float vol) { m_masterVolume = vol; }

    // Portals for acoustic propagation
    void setPortals(const std::vector<Portal>& portals) { m_portals = portals; }

private:
    void playSound(const std::vector<int16_t>& samples, float pan = 0.0f, float volume = 1.0f, bool loop = false);

    QAudioOutput* m_audioOutput = nullptr;
    AudioStreamDevice* m_streamDevice = nullptr;

    std::vector<AudioVoice> m_voices;
    std::mutex m_mutex;

    float m_masterVolume = 1.0f;
    Listener3D m_listener;
    std::vector<Portal> m_portals;

    ReverbManager m_reverb;
    MusicDirector m_musicDirector;
    BodyAudio m_bodyAudio;

    int m_ghostCue = 0;
    bool m_nextDoorOpens = true;
    float m_fearNow = 0.0f;
    float m_staminaNow = 1.0f;
    // music director state
    std::string m_musicName;
    int m_musicTag = 200;
    float m_musicClock = 0.0f;
    float m_musicSwitchedAt = -100.0f;
    float m_musicVolume = 0.0f;
    bool m_assetsAdopted = false;
    float m_ambientCueTimer = 12.0f;
    Vec3 m_listenerPos;
    int m_ctxWing = 0;
    std::vector<std::vector<int16_t>> m_tickCache;   // [speaker*4 + variant]
    int m_tickCounter = 0;
    bool m_gameplayMuted = false;
    float m_bodyAudioTimer = 0.0f;
    float m_breathCooldown = 0.0f;
    float m_musicTimer = 0.0f;
};

} // namespace hm
