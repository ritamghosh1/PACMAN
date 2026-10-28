#include <cassert>
#include <iostream>
#include <cmath>
#include "audio/Synth.h"
#include "audio/Synth2.h"
#include "audio/Spatial.h"
#include "audio/Fdn.h"
#include "audio/EarlyRefl.h"
#include "audio/CineSfx.h"
#include "audio/scores/motifs.h"

using namespace hm;

int main() {
    std::cout << "[TEST] Running Audio QA Test Suite..." << std::endl;

    // Test 1: Synth foley outputs valid non-empty buffer
    auto step = Synth::generateFootstep(0);
    assert(!step.empty());
    for (int16_t s : step) {
        assert(!std::isnan(static_cast<float>(s)));
    }
    std::cout << "  ✓ Synth foley footstep OK (size: " << step.size() << ")" << std::endl;

    // Test 2: Synth2 motif & music-box
    auto motif = Synth2::generateMusicBoxMotif();
    assert(!motif.empty());
    std::cout << "  ✓ Music-box 'Hush Little Light' motif OK (size: " << motif.size() << ")" << std::endl;

    // Test 3: Ghost acoustic signatures
    auto sob = Synth2::generateWeeperSobbing(1.0f);
    assert(!sob.empty());
    auto shears = Synth2::generateHollowManShearsSnip();
    assert(!shears.empty());
    auto choir = Synth2::generateChoirChord(true, 1.0f);
    assert(!choir.empty());
    auto keys = Synth2::generateWardenKeyJingle();
    assert(!keys.empty());
    std::cout << "  ✓ Ghost signatures (Weeper, Hollow Man, Choir, Warden) OK" << std::endl;

    // Test 4: 3D Spatial audio listener panning
    Listener3D listener;
    listener.position = Vec3(0, 0, 0);
    listener.yawDeg = 0.0f;

    Vec3 soundRight(10, 0, 0);
    auto spatRight = Spatial::compute(soundRight, listener, 30.0f);
    assert(spatRight.gainRight > spatRight.gainLeft);

    Vec3 soundLeft(-10, 0, 0);
    auto spatLeft = Spatial::compute(soundLeft, listener, 30.0f);
    assert(spatLeft.gainLeft > spatLeft.gainRight);
    std::cout << "  ✓ Spatial binaural left/right panning OK" << std::endl;

    // Test 5: FDN Reverb
    FdnReverb fdn;
    float outL = 0.0f, outR = 0.0f;
    fdn.processSample(0.8f, outL, outR);
    assert(!std::isnan(outL) && !std::isnan(outR));
    std::cout << "  ✓ FDN 8-line Reverb filter OK" << std::endl;

    // Test 6: cutscene sound design (typewriter ticks, motifs, beds)
    for (int s = 0; s < static_cast<int>(Speaker::Count); ++s) {
        auto tick = cinesfx::typeTick(static_cast<Speaker>(s), 0);
        auto mot = cinesfx::motif(static_cast<Speaker>(s));
        assert(!tick.empty() && !mot.empty());
    }
    assert(!cinesfx::rainBed(2.0f).empty() && !cinesfx::musicBoxLullaby(0.5f).empty());
    std::cout << "  ✓ CineSfx ticks, motifs and beds OK" << std::endl;

    std::cout << "[TEST] ALL AUDIO QA TESTS PASSED!" << std::endl;
    return 0;
}
