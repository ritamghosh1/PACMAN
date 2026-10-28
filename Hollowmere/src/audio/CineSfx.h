#pragma once

#include <vector>
#include <cstdint>

namespace hm {

// Who is "speaking" on screen. Each one gets its own typewriter tick timbre and
// a short signature motif so the player recognises them by ear (no voice acting).
enum class Speaker {
    Narrator,   // typewriter / letters / records
    Nora,
    Mae,
    Crane,
    Ada,
    Eli,
    Manor,      // the house itself, system hints
    Count
};

// Procedural sound design for cutscenes, story text and UI. Everything is mono
// 16-bit PCM at 44.1 kHz. Ambience beds are loop-safe (end crossfaded to start).
namespace cinesfx {

constexpr int SAMPLE_RATE = 44100;

// --- Ambience beds (loop) ---
std::vector<int16_t> rainBed(float sec = 8.0f);
std::vector<int16_t> windBed(float sec = 10.0f);
std::vector<int16_t> fireBed(float sec = 8.0f);
std::vector<int16_t> carBed(float sec = 6.0f);
std::vector<int16_t> darkPad(float rootHz = 55.0f, float sec = 12.0f, float tension = 0.5f);
std::vector<int16_t> graveyardNight(float sec = 10.0f);   // crickets-free: wind + sparse low tones

// --- One-shots ---
std::vector<int16_t> thunder(float sec = 6.5f);
std::vector<int16_t> gateCreak(float sec = 3.4f);
std::vector<int16_t> gateClang(float sec = 3.2f);
std::vector<int16_t> knock(int count = 3);
std::vector<int16_t> paperRustle(float sec = 1.3f);
std::vector<int16_t> heavyThud();
std::vector<int16_t> glassShatter();
std::vector<int16_t> clockTicks(int count = 8, float bpm = 60.0f);
std::vector<int16_t> wiperSwish();
std::vector<int16_t> matchStrike();
std::vector<int16_t> riser(float sec = 4.0f);
std::vector<int16_t> impact();
std::vector<int16_t> doorLockClick();
std::vector<int16_t> windowLightOn();   // soft electric/candle "tung"
std::vector<int16_t> phoneBuzz();
std::vector<int16_t> carDoorSlam();
std::vector<int16_t> footstepsGravel(int steps = 6);

// --- Music box lullaby ("Hush, Little Light"), optionally warped ---
std::vector<int16_t> musicBoxLullaby(float warp = 0.0f, int bars = 8);
std::vector<int16_t> musicBoxNote(float hz, float sec = 1.6f);

// --- Character recognition ---
std::vector<int16_t> typeTick(Speaker who, int variant);   // variant 0..3
std::vector<int16_t> motif(Speaker who);                    // short signature phrase
std::vector<int16_t> carriageDing();

// --- UI ---
std::vector<int16_t> uiHover();
std::vector<int16_t> uiClick();
std::vector<int16_t> uiAdvance();

// Every buffer the story scenes need, rendered once on a worker thread so the
// first New Game never stutters. get() blocks only if rendering has not finished.
struct Bank {
    std::vector<int16_t> rain, wind, fire, car, padLow, padTense, graveyard;
    std::vector<int16_t> thunderA, thunderB, creak, clang, knock, rustle, thud, shatter;
    std::vector<int16_t> clock, wiper, match, riser, impact, lock, windowOn, buzz, carDoor, gravel;
    std::vector<int16_t> boxClean, boxWarped;
    static void prewarm();
    static bool ready();
    static const Bank& get();
};

// Writes a mono WAV for offline inspection.
bool writeWav(const char* path, const std::vector<int16_t>& s);

} // namespace cinesfx
} // namespace hm
