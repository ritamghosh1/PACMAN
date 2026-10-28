#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <cstdint>

namespace hm {

// A decoded recording, resampled to 44.1 kHz. Music stays stereo (interleaved); everything else is mono
// so it can be placed in 3D.
struct AudioClip {
    std::vector<int16_t> pcm;
    bool stereo = false;
    float seconds = 0.0f;
};

// Recordings from assets/audio/*.ogg (credited in assets/audio/ATTRIBUTION.md), decoded on a worker thread.
// Clips live for the whole process, so the mixer can reference them without copying.
class AssetAudio {
public:
    enum class State { NotStarted, Loading, Ready, Missing };

    static AssetAudio& instance();

    void preload();
    State state() const { return m_state.load(); }
    bool ready() const { return m_state.load() == State::Ready; }
    bool missing() const { return m_state.load() == State::Missing; }

    // Exact name (file name without extension). nullptr if absent or still loading.
    const AudioClip* find(const std::string& name);
    // Random clip whose name starts with `prefix` (e.g. "horror_"); avoids repeating the last pick.
    const AudioClip* pick(const std::string& prefix);
    // Resolved assets directory (empty if none was found)
    const std::string& directory() const { return m_dir; }

private:
    AssetAudio() = default;
    void loadAll();
    static std::string locate();

    std::string m_dir;
    std::map<std::string, AudioClip> m_clips;   // std::map: references stay valid while loading continues
    std::map<std::string, std::string> m_lastPick;
    std::mutex m_mutex;
    std::atomic<State> m_state{State::NotStarted};
    unsigned m_rng = 12345u;
};

} // namespace hm
