#include "AssetAudio.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <thread>

extern "C" {
int stb_vorbis_decode_filename(const char* filename, int* channels, int* sample_rate, short** output);
}

#ifndef HM_ASSET_DIR
#define HM_ASSET_DIR ""
#endif

namespace hm {

AssetAudio& AssetAudio::instance() {
    static AssetAudio inst;
    return inst;
}

std::string AssetAudio::locate() {
    QStringList candidates;
    if (const char* env = std::getenv("HOLLOWMERE_ASSETS")) candidates << QString::fromUtf8(env);
    candidates << QString::fromUtf8(HM_ASSET_DIR);
    const QString app = QCoreApplication::applicationDirPath();
    candidates << app + "/assets" << app + "/../assets" << app + "/../../assets" << app + "/../Resources/assets" << "assets" << "../assets";
    for (const QString& c : candidates) {
        if (c.isEmpty()) continue;
        if (QFileInfo(c + "/audio").isDir()) return QDir(c + "/audio").absolutePath().toStdString();
    }
    return {};
}

void AssetAudio::preload() {
    State expected = State::NotStarted;
    if (!m_state.compare_exchange_strong(expected, State::Loading)) return;
    m_dir = locate();
    if (m_dir.empty()) {
        m_state = State::Missing;
        return;
    }
    std::thread([this] { loadAll(); }).detach();
}

void AssetAudio::loadAll() {
    QDir dir(QString::fromStdString(m_dir));
    QStringList files = dir.entryList(QStringList() << "*.ogg", QDir::Files, QDir::Name);
    // Menu music first so the title screen is never silent
    std::stable_sort(files.begin(), files.end(), [](const QString& a, const QString& b) { return (a == "music_menu.ogg") > (b == "music_menu.ogg"); });
    for (const QString& f : files) {
        int ch = 0, sr = 0;
        short* data = nullptr;
        std::string path = dir.absoluteFilePath(f).toStdString();
        int frames = stb_vorbis_decode_filename(path.c_str(), &ch, &sr, &data);
        if (frames <= 0 || !data || ch < 1) { std::free(data); continue; }
        const std::string name = QFileInfo(f).completeBaseName().toStdString();
        const bool music = name.rfind("music_", 0) == 0;
        const int outCh = (music && ch >= 2) ? 2 : 1;

        // Mixdown to mono/stereo as float, then resample to 44.1 kHz with linear interpolation
        std::vector<float> src(static_cast<size_t>(frames) * outCh);
        for (int i = 0; i < frames; ++i) {
            if (outCh == 2) {
                src[2 * i] = data[i * ch] / 32768.0f;
                src[2 * i + 1] = data[i * ch + 1] / 32768.0f;
            } else {
                float s = 0;
                for (int c = 0; c < std::min(ch, 2); ++c) s += data[i * ch + c];
                src[i] = s / (32768.0f * std::min(ch, 2));
            }
        }
        std::free(data);
        const double ratio = 44100.0 / sr;
        const size_t outFrames = static_cast<size_t>(frames * ratio);
        std::vector<float> res(outFrames * outCh);
        float peak = 1e-4f;
        for (size_t i = 0; i < outFrames; ++i) {
            double pos = i / ratio;
            size_t i0 = std::min<size_t>(static_cast<size_t>(pos), frames - 1), i1 = std::min<size_t>(i0 + 1, frames - 1);
            float t = static_cast<float>(pos - i0);
            for (int c = 0; c < outCh; ++c) {
                float v = src[i0 * outCh + c] * (1 - t) + src[i1 * outCh + c] * t;
                res[i * outCh + c] = v;
                peak = std::max(peak, std::fabs(v));
            }
        }
        // Level every clip so volume knobs in the game mean the same thing for all of them
        const float gain = (music ? 0.80f : 0.92f) / peak;
        AudioClip clip;
        clip.stereo = outCh == 2;
        clip.seconds = static_cast<float>(outFrames) / 44100.0f;
        clip.pcm.resize(res.size());
        for (size_t i = 0; i < res.size(); ++i) clip.pcm[i] = static_cast<int16_t>(std::clamp(res[i] * gain, -1.0f, 1.0f) * 32000.0f);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_clips.emplace(name, std::move(clip));
    }
    m_state = m_clips.empty() ? State::Missing : State::Ready;
    qInfo("Audio: decoded %d recordings from %s", static_cast<int>(m_clips.size()), m_dir.c_str());
}

const AudioClip* AssetAudio::find(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_clips.find(name);
    return it == m_clips.end() ? nullptr : &it->second;
}

const AudioClip* AssetAudio::pick(const std::string& prefix) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::map<std::string, AudioClip>::iterator> hits;
    for (auto it = m_clips.lower_bound(prefix); it != m_clips.end() && it->first.compare(0, prefix.size(), prefix) == 0; ++it) hits.push_back(it);
    if (hits.empty()) return nullptr;
    m_rng = m_rng * 1664525u + 1013904223u;
    size_t idx = (m_rng >> 8) % hits.size();
    if (hits.size() > 1 && hits[idx]->first == m_lastPick[prefix]) idx = (idx + 1) % hits.size();
    m_lastPick[prefix] = hits[idx]->first;
    return &hits[idx]->second;
}

} // namespace hm
