#include "CineSfx.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <future>

namespace hm {
namespace cinesfx {
namespace {

constexpr float SR = static_cast<float>(SAMPLE_RATE);
constexpr float TAU = 6.28318530718f;

using Buf = std::vector<float>;

// Small deterministic noise source: sound design must be repeatable between runs.
struct Rnd {
    uint32_t s;
    explicit Rnd(uint32_t seed) : s(seed ? seed : 1u) {}
    float next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (s & 0xFFFFFFu) / 16777216.0f; }
    float bi() { return next() * 2.0f - 1.0f; }
};

Buf mk(float sec) { return Buf(static_cast<size_t>(sec * SR), 0.0f); }
float midiHz(float m) { return 440.0f * std::pow(2.0f, (m - 69.0f) / 12.0f); }
float decay(float t, float tau) { return std::exp(-t / tau); }

// RBJ biquad with retunable coefficients (state survives retuning so sweeps stay click-free).
struct Bq {
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    enum Kind { LP, HP, BP };
    Kind kind;
    Bq(Kind k, float f, float q = 0.707f) : kind(k) { set(f, q); }
    void set(float f, float q) {
        f = std::clamp(f, 20.0f, SR * 0.45f);
        float w = TAU * f / SR, c = std::cos(w), al = std::sin(w) / (2.0f * q);
        float a0 = 1.0f + al;
        switch (kind) {
        case LP: b0 = (1 - c) * 0.5f; b1 = 1 - c; b2 = b0; break;
        case HP: b0 = (1 + c) * 0.5f; b1 = -(1 + c); b2 = b0; break;
        case BP: b0 = al; b1 = 0; b2 = -al; break;
        }
        a1 = -2.0f * c; a2 = 1.0f - al;
        b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
    }
    float p(float x) {
        float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = x; y2 = y1; y1 = y;
        return y;
    }
};

// Paul Kellet pink-noise approximation: rain and wind sit better on 1/f than on white.
struct Pink {
    float b0 = 0, b1 = 0, b2 = 0;
    float p(float w) {
        b0 = 0.99765f * b0 + w * 0.0990460f;
        b1 = 0.96300f * b1 + w * 0.2965164f;
        b2 = 0.57000f * b2 + w * 1.0526913f;
        return (b0 + b1 + b2 + w * 0.1848f) * 0.25f;
    }
};

// Schroeder reverb (6 combs + 2 allpass). Extends the buffer by `tail` seconds.
Buf reverb(const Buf& dryIn, float mix, float decaySec, float tail) {
    Buf in = dryIn;
    size_t fadeN = std::min<size_t>(in.size() / 3, static_cast<size_t>(0.18f * SR));   // never hard-cut the source into the tail
    for (size_t i = 0; i < fadeN; ++i) in[in.size() - 1 - i] *= static_cast<float>(i) / fadeN;
    static const int combs[6] = {1557, 1617, 1491, 1422, 1277, 1356};
    static const int aps[2] = {225, 556};
    size_t n = in.size() + static_cast<size_t>(tail * SR);
    Buf out(n, 0.0f), wet(n, 0.0f);
    for (int c = 0; c < 6; ++c) {
        int d = combs[c];
        float g = std::pow(10.0f, -3.0f * d / (decaySec * SR));
        Buf line(d, 0.0f);
        float lp = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            float x = i < in.size() ? in[i] : 0.0f;
            float y = line[i % d];
            lp = lp * 0.35f + y * 0.65f;   // damping: highs die first, like a real room
            line[i % d] = x + lp * g;
            wet[i] += y * (1.0f / 6.0f);
        }
    }
    for (int a = 0; a < 2; ++a) {
        int d = aps[a];
        Buf line(d, 0.0f);
        for (size_t i = 0; i < n; ++i) {
            float y = line[i % d];
            float x = wet[i];
            line[i % d] = x + y * 0.5f;
            wet[i] = y - x * 0.5f;
        }
    }
    for (size_t i = 0; i < n; ++i) out[i] = (i < in.size() ? in[i] : 0.0f) * (1.0f - mix * 0.5f) + wet[i] * mix * 2.2f;
    return out;
}

// Make a bed loop seamlessly by crossfading its tail into its head.
Buf loopify(const Buf& b, float xfadeSec) {
    size_t xf = static_cast<size_t>(xfadeSec * SR);
    if (b.size() <= xf * 2) return b;
    size_t n = b.size() - xf;
    Buf out(b.begin(), b.begin() + n);
    for (size_t i = 0; i < xf; ++i) {
        float t = static_cast<float>(i) / xf;
        float a = std::sin(t * TAU * 0.25f), c = std::cos(t * TAU * 0.25f);   // equal-power
        out[i] = b[i] * a + b[n + i] * c;
    }
    return out;
}

std::vector<int16_t> pcm(const Buf& b, float peak = 0.85f, bool fade = true) {
    float m = 1e-6f;
    for (float v : b) m = std::max(m, std::fabs(v));
    float g = peak / m;
    std::vector<int16_t> o(b.size());
    size_t f = std::min<size_t>(b.size() / 2, 96);
    for (size_t i = 0; i < b.size(); ++i) {
        float v = b[i] * g;
        if (fade) {
            if (i < f) v *= static_cast<float>(i) / f;
            if (b.size() - i <= f) v *= static_cast<float>(b.size() - i) / f;
        }
        v = std::tanh(v * 1.1f) / std::tanh(1.1f);
        o[i] = static_cast<int16_t>(std::clamp(v, -1.0f, 1.0f) * 32000.0f);
    }
    return o;
}

// Adds an exponentially decaying sine partial.
void partial(Buf& b, float startSec, float hz, float amp, float tau, float phase = 0.0f, float attack = 0.002f) {
    size_t s0 = static_cast<size_t>(startSec * SR);
    size_t len = static_cast<size_t>(tau * 7.0f * SR);
    for (size_t i = 0; i < len && s0 + i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float env = decay(t, tau) * (1.0f - std::exp(-t / attack));
        b[s0 + i] += std::sin(TAU * hz * t + phase) * amp * env;
    }
}

void addAt(Buf& dst, const Buf& src, float startSec, float gain = 1.0f) {
    size_t s0 = static_cast<size_t>(startSec * SR);
    for (size_t i = 0; i < src.size() && s0 + i < dst.size(); ++i) dst[s0 + i] += src[i] * gain;
}

Buf bellStrike(float hz, float sec, float amp, float inharm = 1.0f) {
    Buf b = mk(sec);
    static const float r[5] = {1.0f, 2.32f, 4.25f, 6.63f, 9.38f};
    static const float a[5] = {1.0f, 0.7f, 0.5f, 0.35f, 0.2f};
    static const float t[5] = {1.0f, 0.65f, 0.42f, 0.27f, 0.16f};
    for (int k = 0; k < 5; ++k) partial(b, 0, hz * (1.0f + (r[k] - 1.0f) * inharm), amp * a[k], sec * 0.33f * t[k], k * 0.7f);
    return b;
}

} // namespace

// ============================================================================
// Ambience beds
// ============================================================================

std::vector<int16_t> rainBed(float sec) {
    Buf b = mk(sec + 0.6f);
    Rnd r(11);
    Pink pk;
    Bq hp(Bq::HP, 140.0f), lp(Bq::LP, 2600.0f), roof(Bq::BP, 1500.0f, 0.8f);   // soft low patter: no harsh hiss above 3 kHz
    for (size_t i = 0; i < b.size(); ++i) {
        float w = r.bi();
        float x = lp.p(hp.p(pk.p(w))) * 1.6f + roof.p(w) * 0.08f;
        b[i] = x;
    }
    // Individual drops: tiny pitched ticks sprinkled irregularly.
    for (int d = 0; d < static_cast<int>(sec * 14); ++d) {
        float t0 = r.next() * sec;
        partial(b, t0, 900.0f + r.next() * 1200.0f, 0.03f + r.next() * 0.03f, 0.008f + r.next() * 0.01f);
    }
    return pcm(loopify(b, 0.6f), 0.5f, false);
}

std::vector<int16_t> windBed(float sec) {
    Buf b = mk(sec + 1.0f);
    Rnd r(23);
    Bq bp1(Bq::BP, 400.0f, 2.2f), bp2(Bq::BP, 900.0f, 3.0f), lo(Bq::LP, 180.0f);
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        if ((i & 63) == 0) {
            bp1.set(330.0f + 160.0f * std::sin(TAU * 0.11f * t) + 90.0f * std::sin(TAU * 0.27f * t + 1.0f), 2.2f);
            bp2.set(820.0f + 280.0f * std::sin(TAU * 0.07f * t + 2.0f), 3.2f);
        }
        float gust = 0.55f + 0.45f * std::sin(TAU * 0.09f * t) * std::sin(TAU * 0.23f * t + 0.5f);
        float w = r.bi();
        b[i] = (bp1.p(w) * 1.3f + bp2.p(w) * 0.8f) * gust + lo.p(w) * 0.9f;
        b[i] += std::sin(TAU * (1250.0f + 40.0f * std::sin(TAU * 0.3f * t)) * t) * 0.012f * gust * gust;   // thin whistle in the eaves
    }
    return pcm(loopify(b, 1.0f), 0.55f, false);
}

std::vector<int16_t> fireBed(float sec) {
    Buf b = mk(sec + 0.6f);
    Rnd r(37);
    Bq rumble(Bq::LP, 140.0f), body(Bq::BP, 500.0f, 0.6f), hiss(Bq::HP, 3000.0f);
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float swell = 0.65f + 0.35f * std::sin(TAU * 0.6f * t) * std::sin(TAU * 0.17f * t + 1.0f);
        float w = r.bi();
        b[i] = rumble.p(w) * 3.0f * swell + body.p(w) * 0.8f * swell + hiss.p(w) * 0.05f * swell;
    }
    // Crackles and pops: short bright bursts of random strength.
    Bq cr(Bq::BP, 2400.0f, 0.9f);
    for (int k = 0; k < static_cast<int>(sec * 38); ++k) {
        float t0 = r.next() * sec;
        size_t s0 = static_cast<size_t>(t0 * SR);
        size_t len = static_cast<size_t>((0.002f + r.next() * 0.012f) * SR);
        float amp = std::pow(r.next(), 2.0f) * 0.7f + 0.12f;
        cr.set(1200.0f + r.next() * 3500.0f, 1.2f);
        for (size_t i = 0; i < len && s0 + i < b.size(); ++i)
            b[s0 + i] += cr.p(r.bi()) * amp * (1.0f - static_cast<float>(i) / len);
    }
    return pcm(loopify(b, 0.6f), 0.62f, false);
}

std::vector<int16_t> carBed(float sec) {
    Buf b = mk(sec + 0.5f);
    Rnd r(41);
    Bq engLp(Bq::LP, 420.0f, 0.9f), road(Bq::LP, 520.0f), roof(Bq::BP, 3200.0f, 0.7f);
    float ph = 0.0f;
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float rpm = 46.0f + 3.0f * std::sin(TAU * 0.4f * t) + 1.2f * std::sin(TAU * 1.3f * t);
        ph += TAU * rpm / SR;
        float e = 0.0f;
        for (int k = 1; k <= 9; ++k) e += std::sin(ph * k + k) / static_cast<float>(k);
        float w = r.bi();
        b[i] = engLp.p(e) * 0.9f + road.p(w) * 0.45f + roof.p(w) * 0.07f * (0.7f + 0.3f * std::sin(TAU * 7.0f * t));
        float seam = std::fmod(t, 0.92f);   // tyre thump over road seams
        b[i] += std::sin(TAU * 58.0f * seam) * decay(seam, 0.05f) * 0.35f;
    }
    return pcm(loopify(b, 0.5f), 0.58f, false);
}

std::vector<int16_t> darkPad(float rootHz, float sec, float tension) {
    Buf b = mk(sec + 1.5f);
    Rnd r(53);
    Bq lp1(Bq::LP, 300.0f, 0.8f);
    float ph[6] = {0, 1, 2, 3, 4, 5};
    float mult[6] = {1.0f, 1.0058f, 0.9942f, 1.4983f * 2.0f, 1.0595f * 2.0f, 1.4142f * 2.0f};
    float amp[6] = {1.0f, 0.7f, 0.7f, 0.25f, 0.30f * tension, 0.22f * tension};
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        if ((i & 127) == 0) lp1.set(220.0f + 180.0f * (0.5f + 0.5f * std::sin(TAU * 0.06f * t)) + 120.0f * tension, 0.8f);
        float s = 0.0f;
        for (int k = 0; k < 6; ++k) {
            ph[k] += TAU * rootHz * mult[k] / SR;
            float saw = std::fmod(ph[k], TAU) / TAU * 2.0f - 1.0f;
            s += saw * amp[k];
        }
        float sub = std::sin(TAU * rootHz * 0.5f * t) * 0.9f;
        float breath = 0.75f + 0.25f * std::sin(TAU * 0.09f * t + 0.3f);
        b[i] = (lp1.p(s * 0.35f) + sub * 0.5f) * breath + r.bi() * 0.004f;
    }
    return pcm(loopify(b, 1.5f), 0.5f, false);
}

std::vector<int16_t> graveyardNight(float sec) {
    Buf wind;
    {
        auto w = windBed(sec);
        wind.resize(w.size());
        for (size_t i = 0; i < w.size(); ++i) wind[i] = w[i] / 32768.0f * 0.55f;
    }
    Buf bell = bellStrike(98.0f, 5.0f, 0.12f, 0.6f);
    addAt(wind, bell, 2.0f);
    addAt(wind, bell, sec * 0.62f, 0.7f);
    return pcm(wind, 0.5f, false);
}

// ============================================================================
// One-shots
// ============================================================================

std::vector<int16_t> thunder(float sec) {
    Buf b = mk(sec);
    Rnd r(61);
    Bq crackBp(Bq::BP, 2200.0f, 0.5f), rumbleLp(Bq::LP, 700.0f, 0.7f);
    struct Swell { float t, tau, amp; };
    Swell sw[6] = {{0.0f, 0.9f, 1.0f}, {0.45f, 1.3f, 0.8f}, {1.2f, 1.6f, 0.7f}, {2.2f, 1.9f, 0.55f}, {3.1f, 1.4f, 0.4f}, {3.9f, 1.8f, 0.3f}};
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float w = r.bi();
        float crack = crackBp.p(w) * decay(t, 0.06f) * 2.0f;
        if ((i & 127) == 0) rumbleLp.set(std::max(70.0f, 750.0f * std::exp(-t * 0.8f)), 0.7f);
        float env = 0.0f;
        for (const auto& s : sw) if (t >= s.t) env += s.amp * decay(t - s.t, s.tau) * (1.0f - std::exp(-(t - s.t) / 0.12f));
        b[i] = crack + rumbleLp.p(w) * env * 3.2f;
    }
    return pcm(reverb(b, 0.25f, 2.2f, 1.2f), 0.9f);
}

std::vector<int16_t> gateCreak(float sec) {
    Buf b = mk(sec);
    Rnd r(71);
    Bq jit(Bq::LP, 9.0f), f1(Bq::BP, 700.0f, 6.0f), f2(Bq::BP, 1650.0f, 8.0f), f3(Bq::BP, 3100.0f, 10.0f);
    float ph = 0.0f, ph2 = 0.0f;
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float j = jit.p(r.bi()) * 6.0f;
        float hz = 170.0f + 110.0f * std::sin(TAU * 0.29f * t) + 55.0f * t / sec + 38.0f * j;
        ph += TAU * hz / SR;
        ph2 += TAU * hz * 3.01f / SR;
        float saw = std::fmod(ph, TAU) / TAU * 2.0f - 1.0f;
        float sq = std::fmod(ph2, TAU) / TAU > 0.5f ? 1.0f : -1.0f;
        float src = saw + sq * 0.25f;
        float stick = 0.5f + 0.5f * std::sin(TAU * 11.0f * t + 3.0f * j);   // stick-slip judder
        float env = std::min(1.0f, t / 0.25f) * std::min(1.0f, (sec - t) / 0.7f);
        b[i] = (f1.p(src) + f2.p(src) * 0.8f + f3.p(src) * 0.35f) * (0.35f + 0.65f * stick) * env;
        float sq2 = std::sin(TAU * (2300.0f + 500.0f * std::sin(TAU * 0.5f * t)) * t) * 0.07f;
        b[i] += sq2 * env * (j > 0.35f ? 1.0f : 0.0f);
    }
    return pcm(reverb(b, 0.2f, 1.6f, 0.8f), 0.8f);
}

std::vector<int16_t> gateClang(float sec) {
    Buf b = mk(sec);
    Rnd r(83);
    Buf bell = bellStrike(210.0f, sec, 0.9f, 1.0f);
    Bq tr(Bq::BP, 3000.0f, 0.6f);
    for (size_t i = 0; i < b.size() && i < static_cast<size_t>(0.05f * SR); ++i)
        b[i] += tr.p(r.bi()) * (1.0f - i / (0.05f * SR)) * 1.6f;
    for (size_t i = 0; i < b.size() && i < bell.size(); ++i) b[i] += bell[i];
    Buf thump = mk(0.5f);
    partial(thump, 0, 60.0f, 1.4f, 0.14f);
    addAt(b, thump, 0.0f);
    return pcm(reverb(b, 0.3f, 2.0f, 1.0f), 0.95f);
}

std::vector<int16_t> knock(int count) {
    Buf b = mk(0.5f * count + 0.6f);
    Rnd r(97);
    for (int k = 0; k < count; ++k) {
        float t0 = k * 0.46f + (k == 2 ? 0.08f : 0.0f);
        float f = 118.0f - k * 6.0f;
        partial(b, t0, f, 1.0f, 0.045f);
        partial(b, t0, f * 2.4f, 0.4f, 0.025f);
        Bq bp(Bq::BP, 1800.0f, 1.0f);
        size_t s0 = static_cast<size_t>(t0 * SR);
        for (size_t i = 0; i < 400 && s0 + i < b.size(); ++i) b[s0 + i] += bp.p(r.bi()) * (1.0f - i / 400.0f) * 0.8f;
    }
    return pcm(reverb(b, 0.22f, 1.2f, 0.7f), 0.9f);
}

std::vector<int16_t> paperRustle(float sec) {
    Buf b = mk(sec);
    Rnd r(101);
    Bq bp(Bq::BP, 4200.0f, 0.7f), hp(Bq::HP, 1500.0f);
    float env = 0.0f;
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        if ((i % 700) == 0) env = r.next() > 0.45f ? r.next() : 0.0f;   // crinkly stop-start
        float target = env * std::sin(std::min(1.0f, t / 0.1f) * 1.57f) * std::min(1.0f, (sec - t) / 0.3f);
        b[i] = hp.p(bp.p(r.bi())) * target;
    }
    return pcm(b, 0.55f);
}

std::vector<int16_t> heavyThud() {
    Buf b = mk(1.6f);
    Rnd r(109);
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        b[i] = std::sin(TAU * (80.0f * std::exp(-t * 6.0f) + 38.0f) * t) * decay(t, 0.22f) * 1.3f;
    }
    Bq lp(Bq::LP, 600.0f);
    for (size_t i = 0; i < 4000; ++i) b[i] += lp.p(r.bi()) * (1.0f - i / 4000.0f);
    return pcm(reverb(b, 0.2f, 1.4f, 0.5f), 0.95f);
}

std::vector<int16_t> glassShatter() {
    Buf b = mk(2.0f);
    Rnd r(113);
    Bq hp(Bq::HP, 3200.0f);
    for (size_t i = 0; i < 4500; ++i) b[i] += hp.p(r.bi()) * (1.0f - i / 4500.0f) * 1.2f;
    for (int k = 0; k < 90; ++k) {
        float t0 = std::pow(r.next(), 1.8f) * 1.5f;
        partial(b, t0, 2200.0f + r.next() * 6800.0f, 0.12f + r.next() * 0.2f, 0.02f + r.next() * 0.12f);
    }
    return pcm(reverb(b, 0.15f, 1.0f, 0.6f), 0.85f);
}

std::vector<int16_t> clockTicks(int count, float bpm) {
    float step = 60.0f / bpm;
    Buf b = mk(step * count + 0.5f);
    Rnd r(127);
    for (int k = 0; k < count; ++k) {
        float t0 = k * step;
        float f = (k & 1) ? 1050.0f : 1300.0f;
        partial(b, t0, f, 0.8f, 0.016f);
        partial(b, t0, f * 2.1f, 0.35f, 0.01f);
        Bq bp(Bq::BP, 2600.0f, 1.2f);
        size_t s0 = static_cast<size_t>(t0 * SR);
        for (size_t i = 0; i < 260 && s0 + i < b.size(); ++i) b[s0 + i] += bp.p(r.bi()) * (1.0f - i / 260.0f) * 0.8f;
    }
    return pcm(reverb(b, 0.12f, 0.9f, 0.4f), 0.7f);
}

std::vector<int16_t> wiperSwish() {
    Buf b = mk(1.1f);
    Rnd r(131);
    Bq bp(Bq::BP, 1800.0f, 0.9f);
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float env = std::sin(std::min(1.0f, t / 0.8f) * 3.14159f);
        if ((i & 255) == 0) bp.set(1400.0f + 900.0f * t, 0.9f);
        b[i] = bp.p(r.bi()) * env * 0.9f;
    }
    partial(b, 0.85f, 95.0f, 0.5f, 0.05f);
    return pcm(b, 0.5f);
}

std::vector<int16_t> matchStrike() {
    Buf b = mk(1.6f);
    Rnd r(137);
    Bq hp(Bq::HP, 2500.0f), lp(Bq::LP, 3500.0f), cr(Bq::BP, 3000.0f, 1.0f);
    for (size_t i = 0; i < static_cast<size_t>(0.22f * SR); ++i) {
        float t = static_cast<float>(i) / SR;
        b[i] += hp.p(r.bi()) * (0.3f + t * 3.0f);   // scrape gets sharper before ignition
    }
    size_t s0 = static_cast<size_t>(0.22f * SR);
    for (size_t i = 0; s0 + i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float env = (1.0f - std::exp(-t / 0.03f)) * decay(t, 0.28f);
        b[s0 + i] += lp.p(r.bi()) * env * 1.4f;
        if (r.next() < 0.004f) b[s0 + i] += cr.p(r.bi()) * 0.8f * decay(t, 0.4f);
    }
    return pcm(b, 0.7f);
}

std::vector<int16_t> riser(float sec) {
    Buf b = mk(sec);
    Rnd r(139);
    Bq bp(Bq::BP, 200.0f, 1.5f);
    float ph = 0.0f, ph2 = 0.0f;
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float u = t / sec;
        if ((i & 63) == 0) bp.set(180.0f + 5200.0f * u * u, 1.5f);
        ph += TAU * (70.0f + 380.0f * u * u) / SR;
        ph2 += TAU * (70.0f + 380.0f * u * u) * 1.0595f / SR;   // minor-second rub
        float env = std::pow(u, 2.2f) * std::min(1.0f, (sec - t) / 0.05f);
        b[i] = (bp.p(r.bi()) * 1.3f + std::sin(ph) * 0.5f + std::sin(ph2) * 0.4f) * env;
    }
    return pcm(b, 0.8f);
}

std::vector<int16_t> impact() {
    Buf b = mk(3.0f);
    Rnd r(149);
    Bq lp(Bq::LP, 500.0f);
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        b[i] = std::sin(TAU * (52.0f * std::exp(-t * 1.6f) + 28.0f) * t) * decay(t, 0.7f) * 1.5f;
        if (i < 14000) b[i] += lp.p(r.bi()) * (1.0f - i / 14000.0f) * 1.4f;
    }
    return pcm(reverb(b, 0.3f, 2.5f, 1.5f), 0.95f);
}

std::vector<int16_t> doorLockClick() {
    Buf b = mk(0.6f);
    partial(b, 0.0f, 1800.0f, 0.7f, 0.012f);
    partial(b, 0.0f, 3300.0f, 0.4f, 0.008f);
    partial(b, 0.13f, 1400.0f, 0.8f, 0.016f);
    partial(b, 0.13f, 2650.0f, 0.4f, 0.01f);
    partial(b, 0.0f, 140.0f, 0.5f, 0.03f);
    return pcm(reverb(b, 0.12f, 0.6f, 0.3f), 0.7f);
}

std::vector<int16_t> windowLightOn() {
    Buf b = mk(1.8f);
    Rnd r(151);
    partial(b, 0.0f, 660.0f, 0.7f, 0.5f, 0, 0.004f);
    partial(b, 0.0f, 990.0f, 0.35f, 0.35f);
    partial(b, 0.0f, 1320.0f, 0.15f, 0.2f);
    Bq hp(Bq::HP, 4000.0f);
    for (size_t i = 0; i < 2600; ++i) b[i] += hp.p(r.bi()) * 0.25f * (1.0f - i / 2600.0f);
    return pcm(reverb(b, 0.3f, 1.8f, 0.8f), 0.55f);
}

std::vector<int16_t> phoneBuzz() {
    Buf b = mk(1.5f);
    Rnd r(181);
    Bq lp(Bq::LP, 900.0f);
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float on = std::fmod(t, 0.5f) < 0.28f ? 1.0f : 0.0f;
        float env = on * std::min(1.0f, std::fmod(t, 0.5f) / 0.01f);
        float sq = std::sin(TAU * 172.0f * t) > 0 ? 1.0f : -1.0f;
        b[i] = lp.p(sq * 0.7f + r.bi() * 0.25f) * env;
    }
    return pcm(reverb(b, 0.1f, 0.5f, 0.2f), 0.6f);   // buzzing against a hard surface
}

std::vector<int16_t> carDoorSlam() {
    Buf b = mk(1.2f);
    Rnd r(191);
    Bq lp(Bq::LP, 450.0f), mid(Bq::BP, 1100.0f, 1.2f);
    for (size_t i = 0; i < b.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float w = r.bi();
        b[i] = lp.p(w) * decay(t, 0.09f) * 2.0f + mid.p(w) * decay(t, 0.03f) * 0.8f + std::sin(TAU * 70.0f * t) * decay(t, 0.12f);
    }
    partial(b, 0.0f, 2300.0f, 0.2f, 0.01f);
    return pcm(reverb(b, 0.2f, 1.0f, 0.5f), 0.8f);
}

std::vector<int16_t> footstepsGravel(int steps) {
    Buf b = mk(steps * 0.62f + 0.4f);
    Rnd r(193);
    Bq bp(Bq::BP, 2200.0f, 0.5f), lo(Bq::LP, 300.0f);
    for (int k = 0; k < steps; ++k) {
        size_t s0 = static_cast<size_t>((k * 0.62f + r.next() * 0.04f) * SR);
        size_t len = static_cast<size_t>(0.22f * SR);
        for (size_t i = 0; i < len && s0 + i < b.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float w = r.bi();
            float crunch = bp.p(w) * (r.next() > 0.55f ? 1.0f : 0.25f) * decay(t, 0.07f);
            b[s0 + i] += crunch * 0.7f + lo.p(w) * decay(t, 0.05f) * 0.9f;
        }
    }
    return pcm(reverb(b, 0.12f, 0.9f, 0.3f), 0.55f);
}

// ============================================================================
// Music box
// ============================================================================

std::vector<int16_t> musicBoxNote(float hz, float sec) {
    Buf b = mk(sec);
    static const float ratio[4] = {1.0f, 2.01f, 3.99f, 6.2f};
    static const float amp[4] = {1.0f, 0.38f, 0.2f, 0.08f};
    static const float tau[4] = {0.55f, 0.28f, 0.14f, 0.07f};
    for (int k = 0; k < 4; ++k) partial(b, 0.0f, hz * ratio[k], amp[k], tau[k] * sec * 0.6f, 0.0f, 0.0015f);
    Rnd r(static_cast<uint32_t>(hz));
    Bq hp(Bq::HP, 5000.0f);
    for (size_t i = 0; i < 90; ++i) b[i] += hp.p(r.bi()) * 0.5f * (1.0f - i / 90.0f);   // comb-tine pluck
    return pcm(b, 0.6f);
}

std::vector<int16_t> musicBoxLullaby(float warp, int bars) {
    struct N { float midi, beats; };
    static const N tune[] = {
        {74, 1}, {77, 1}, {81, 1},      {81, 1.5f}, {79, .5f}, {77, 1},
        {76, 1}, {79, 1}, {82, 1},      {81, 3},
        {74, 1}, {77, 1}, {81, 1},      {86, 1.5f}, {84, .5f}, {81, 1},
        {79, 1}, {77, 1}, {76, 1},      {74, 3},
    };
    float beat = 60.0f / 84.0f * (1.0f + warp * 0.55f);
    float total = 0.0f;
    for (const auto& n : tune) total += n.beats;
    float sec = std::min<float>(total, bars * 3.0f) * beat + 3.0f;
    Buf b = mk(sec);
    Rnd r(157);
    float t = 0.0f, used = 0.0f;
    int idx = 0;
    for (const auto& n : tune) {
        if (used >= bars * 3.0f) break;
        bool drop = warp > 0.0f && r.next() < warp * 0.22f && idx > 2;   // a worn comb skips tines
        float cents = warp * (r.bi() * 28.0f - 10.0f) + (idx > 14 ? warp * -35.0f : 0.0f);
        if (!drop) {
            auto note = musicBoxNote(midiHz(n.midi) * std::pow(2.0f, cents / 1200.0f), 2.4f);
            Buf f(note.size());
            for (size_t i = 0; i < note.size(); ++i) f[i] = note[i] / 32768.0f;
            addAt(b, f, t);
        }
        t += n.beats * beat * (1.0f + warp * 0.06f * r.bi());
        used += n.beats;
        ++idx;
    }
    return pcm(reverb(b, 0.35f, 2.2f, 1.2f), 0.7f);
}

// ============================================================================
// Character recognition
// ============================================================================

std::vector<int16_t> typeTick(Speaker who, int variant) {
    int v = variant & 3;
    float vf = 1.0f + 0.045f * (v - 1.5f);
    Rnd r(1000 + static_cast<uint32_t>(who) * 17 + v);
    Buf b = mk(0.12f);
    auto click = [&](float hz, float q, size_t len, float amp) {
        Bq bp(Bq::BP, hz, q);
        for (size_t i = 0; i < len && i < b.size(); ++i) b[i] += bp.p(r.bi()) * (1.0f - static_cast<float>(i) / len) * amp;
    };
    switch (who) {
    case Speaker::Narrator:    // typewriter key strike
        click(3200.0f * vf, 1.2f, 180, 1.0f);
        partial(b, 0.0f, 1750.0f * vf, 0.5f, 0.012f);
        partial(b, 0.0f, 210.0f * vf, 0.55f, 0.03f);
        break;
    case Speaker::Nora:        // soft felt tap, warm and close
        partial(b, 0.0f, 480.0f * vf, 0.9f, 0.022f, 0, 0.003f);
        partial(b, 0.0f, 960.0f * vf, 0.25f, 0.015f);
        click(900.0f, 0.8f, 120, 0.25f);
        break;
    case Speaker::Mae:         // tiny glass/music-box ping
        partial(b, 0.0f, 1760.0f * vf, 0.8f, 0.05f, 0, 0.001f);
        partial(b, 0.0f, 2637.0f * vf, 0.35f, 0.035f);
        break;
    case Speaker::Crane:       // cold, heavy metal clack
        click(900.0f * vf, 1.5f, 260, 1.0f);
        partial(b, 0.0f, 140.0f * vf, 0.9f, 0.04f);
        partial(b, 0.0f, 423.0f * vf, 0.4f, 0.05f);
        break;
    case Speaker::Ada:         // airy breath puff
        click(1400.0f * vf, 0.6f, 1100, 0.6f);
        partial(b, 0.0f, 600.0f * vf, 0.12f, 0.03f);
        break;
    case Speaker::Eli:         // wooden toy block
        partial(b, 0.0f, 880.0f * vf, 0.8f, 0.014f);
        partial(b, 0.0f, 1330.0f * vf, 0.4f, 0.01f);
        click(2000.0f, 1.0f, 110, 0.5f);
        break;
    case Speaker::Manor: default:   // the house: dull wooden knock
        partial(b, 0.0f, 92.0f * vf, 1.0f, 0.05f);
        click(500.0f, 0.8f, 200, 0.4f);
        break;
    }
    return pcm(b, 0.38f);
}

std::vector<int16_t> carriageDing() {
    Buf b = mk(1.4f);
    partial(b, 0.0f, 2350.0f, 0.8f, 0.5f, 0, 0.001f);
    partial(b, 0.0f, 4710.0f, 0.2f, 0.25f);
    return pcm(b, 0.4f);
}

std::vector<int16_t> motif(Speaker who) {
    Buf b = mk(2.2f);
    Rnd r(2000 + static_cast<uint32_t>(who));
    switch (who) {
    case Speaker::Nora: {      // felt piano: D4 then A4, warm and unsure
        const float notes[2] = {293.66f, 440.0f};
        for (int k = 0; k < 2; ++k) {
            float t0 = k * 0.42f;
            partial(b, t0, notes[k], 0.8f, 0.9f, 0, 0.01f);
            partial(b, t0, notes[k] * 2.0f, 0.3f, 0.5f, 0, 0.01f);
            partial(b, t0, notes[k] * 3.0f, 0.1f, 0.3f);
        }
        break;
    }
    case Speaker::Mae: {       // music box E-G-B, last tine sags flat
        const float notes[3] = {659.25f, 783.99f, 987.77f * 0.985f};
        for (int k = 0; k < 3; ++k) {
            auto n = musicBoxNote(notes[k], 1.6f);
            Buf f(n.size());
            for (size_t i = 0; i < n.size(); ++i) f[i] = n[i] / 32768.0f;
            addAt(b, f, k * 0.24f);
        }
        break;
    }
    case Speaker::Crane: {     // tritone drone swelling under a jangle of keys
        Bq lp(Bq::LP, 420.0f);
        float p1 = 0, p2 = 0;
        for (size_t i = 0; i < b.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            p1 += TAU * 73.42f / SR; p2 += TAU * 103.83f / SR;
            float env = std::min(1.0f, t / 0.5f) * decay(std::max(0.0f, t - 0.7f), 0.5f);
            float s1 = std::fmod(p1, TAU) / TAU * 2 - 1, s2 = std::fmod(p2, TAU) / TAU * 2 - 1;
            b[i] += lp.p(s1 + s2 * 0.8f) * env * 0.8f;
        }
        for (int k = 0; k < 7; ++k)
            partial(b, 0.12f + k * 0.07f + r.next() * 0.04f, 2200.0f + r.next() * 3200.0f, 0.18f, 0.12f + r.next() * 0.08f);
        break;
    }
    case Speaker::Narrator: {  // a typewriter bell over a paper rustle
        Buf d = mk(1.4f);
        partial(d, 0.0f, 2350.0f, 0.8f, 0.5f, 0, 0.001f);
        partial(d, 0.0f, 4710.0f, 0.2f, 0.25f);
        addAt(b, d, 0.1f);
        auto rus = paperRustle(0.6f);
        Buf f(rus.size());
        for (size_t i = 0; i < rus.size(); ++i) f[i] = rus[i] / 32768.0f * 0.6f;
        addAt(b, f, 0.0f);
        break;
    }
    case Speaker::Ada: {       // thin tone that slides down and wavers like a held sob
        float ph = 0;
        for (size_t i = 0; i < static_cast<size_t>(1.4f * SR); ++i) {
            float t = static_cast<float>(i) / SR;
            float hz = (680.0f - 230.0f * std::min(1.0f, t / 1.2f)) * (1.0f + 0.02f * std::sin(TAU * 5.6f * t) * std::min(1.0f, t));
            ph += TAU * hz / SR;
            float env = std::sin(std::min(1.0f, t / 1.4f) * 3.14159f);
            b[i] = (std::sin(ph) + 0.3f * std::sin(ph * 2.0f)) * env * 0.5f;
        }
        Bq bp(Bq::BP, 1300.0f, 0.7f);
        for (size_t i = 0; i < static_cast<size_t>(1.4f * SR); ++i) {
            float t = static_cast<float>(i) / SR;
            b[i] += bp.p(r.bi()) * 0.1f * std::sin(std::min(1.0f, t / 1.4f) * 3.14159f);
        }
        break;
    }
    case Speaker::Eli: {       // two toy-piano plinks and a block clack
        const float notes[2] = {1046.5f, 783.99f};
        for (int k = 0; k < 2; ++k) {
            auto n = musicBoxNote(notes[k], 1.0f);
            Buf f(n.size());
            for (size_t i = 0; i < n.size(); ++i) f[i] = n[i] / 32768.0f;
            addAt(b, f, k * 0.3f);
        }
        partial(b, 0.75f, 880.0f, 0.6f, 0.014f);
        partial(b, 0.9f, 880.0f, 0.5f, 0.014f);
        break;
    }
    case Speaker::Manor: default: {  // a low house-sized heartbeat with a distant creak
        partial(b, 0.0f, 55.0f, 1.0f, 0.3f);
        partial(b, 0.32f, 48.0f, 0.8f, 0.3f);
        auto cr = gateCreak(1.1f);
        Buf f(cr.size());
        for (size_t i = 0; i < cr.size(); ++i) f[i] = cr[i] / 32768.0f * 0.25f;
        addAt(b, f, 0.6f);
        break;
    }
    }
    return pcm(reverb(b, 0.3f, 1.8f, 0.5f), 0.7f);
}

// ============================================================================
// UI
// ============================================================================

std::vector<int16_t> uiHover() {
    Buf b = mk(0.09f);
    partial(b, 0.0f, 900.0f, 0.6f, 0.012f);
    return pcm(b, 0.22f);
}

std::vector<int16_t> uiClick() {
    Buf b = mk(0.2f);
    Rnd r(167);
    partial(b, 0.0f, 180.0f, 0.9f, 0.04f);
    partial(b, 0.0f, 1500.0f, 0.4f, 0.03f);
    Bq bp(Bq::BP, 2400.0f, 1.0f);
    for (size_t i = 0; i < 200; ++i) b[i] += bp.p(r.bi()) * (1.0f - i / 200.0f) * 0.6f;
    return pcm(b, 0.45f);
}

std::vector<int16_t> uiAdvance() {
    Buf b = mk(0.12f);
    Rnd r(173);
    Bq bp(Bq::BP, 3000.0f, 0.8f);
    for (size_t i = 0; i < 700; ++i) b[i] += bp.p(r.bi()) * (1.0f - i / 700.0f);
    partial(b, 0.0f, 700.0f, 0.35f, 0.012f);
    return pcm(b, 0.3f);
}

namespace {
Bank* g_bank = nullptr;
std::shared_future<void>& bankFuture() {
    static std::shared_future<void> f;
    return f;
}
void renderBank(Bank& b) {
    b.rain = rainBed(8.0f); b.wind = windBed(10.0f); b.fire = fireBed(8.0f); b.car = carBed(6.0f);
    b.padLow = darkPad(55.0f, 12.0f, 0.3f); b.padTense = darkPad(49.0f, 12.0f, 1.0f); b.graveyard = graveyardNight(10.0f);
    b.thunderA = thunder(6.5f); b.thunderB = thunder(5.0f); b.creak = gateCreak(3.4f); b.clang = gateClang(3.2f);
    b.knock = knock(3); b.rustle = paperRustle(1.3f); b.thud = heavyThud(); b.shatter = glassShatter();
    b.clock = clockTicks(10, 60.0f); b.wiper = wiperSwish(); b.match = matchStrike(); b.riser = riser(4.0f);
    b.impact = impact(); b.lock = doorLockClick(); b.windowOn = windowLightOn(); b.buzz = phoneBuzz();
    b.carDoor = carDoorSlam(); b.gravel = footstepsGravel(8);
    b.boxClean = musicBoxLullaby(0.0f, 4); b.boxWarped = musicBoxLullaby(0.85f, 6);
}
} // namespace

void Bank::prewarm() {
    if (bankFuture().valid()) return;
    g_bank = new Bank();   // lives for the whole process
    bankFuture() = std::async(std::launch::async, [] { renderBank(*g_bank); }).share();
}

bool Bank::ready() {
    return bankFuture().valid() && bankFuture().wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

const Bank& Bank::get() {
    prewarm();
    bankFuture().wait();
    return *g_bank;
}

bool writeWav(const char* path, const std::vector<int16_t>& s) {
    FILE* f = std::fopen(path, "wb");
    if (!f) return false;
    uint32_t dataBytes = static_cast<uint32_t>(s.size() * 2), rate = SAMPLE_RATE, byteRate = rate * 2, riff = 36 + dataBytes, fmtLen = 16;
    uint16_t pcmFmt = 1, ch = 1, align = 2, bits = 16;
    std::fwrite("RIFF", 1, 4, f); std::fwrite(&riff, 4, 1, f); std::fwrite("WAVEfmt ", 1, 8, f);
    std::fwrite(&fmtLen, 4, 1, f); std::fwrite(&pcmFmt, 2, 1, f); std::fwrite(&ch, 2, 1, f);
    std::fwrite(&rate, 4, 1, f); std::fwrite(&byteRate, 4, 1, f); std::fwrite(&align, 2, 1, f);
    std::fwrite(&bits, 2, 1, f); std::fwrite("data", 1, 4, f); std::fwrite(&dataBytes, 4, 1, f);
    std::fwrite(s.data(), 2, s.size(), f);
    std::fclose(f);
    return true;
}

} // namespace cinesfx
} // namespace hm
