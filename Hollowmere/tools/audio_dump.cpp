#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include "audio/Synth.h"
#include "audio/Synth2.h"
#include "audio/scores/motifs.h"
#include "audio/CineSfx.h"
#include "audio/CineSfx.h"

static void writeWav(const std::string& path, const std::vector<int16_t>& pcm, int sampleRate = 44100, int channels = 1) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open " << path << " for writing" << std::endl;
        return;
    }

    int dataSize = static_cast<int>(pcm.size() * sizeof(int16_t));
    int fileSize = 36 + dataSize;

    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char*>(&fileSize), 4);
    out.write("WAVEfmt ", 8);
    int subchunk1Size = 16;
    short audioFormat = 1; // PCM
    short numChannels = static_cast<short>(channels);
    int byteRate = sampleRate * channels * sizeof(int16_t);
    short blockAlign = static_cast<short>(channels * sizeof(int16_t));
    short bitsPerSample = 16;
    out.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    out.write(reinterpret_cast<const char*>(&audioFormat), 2);
    out.write(reinterpret_cast<const char*>(&numChannels), 2);
    out.write(reinterpret_cast<const char*>(&sampleRate), 4);
    out.write(reinterpret_cast<const char*>(&byteRate), 4);
    out.write(reinterpret_cast<const char*>(&blockAlign), 2);
    out.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    out.write("data", 4);
    out.write(reinterpret_cast<const char*>(&dataSize), 4);
    out.write(reinterpret_cast<const char*>(pcm.data()), dataSize);
}

int main() {
    std::string outDir = "audio_samples";
    mkdir(outDir.c_str(), 0755);

    std::cout << "=========================================================" << std::endl;
    std::cout << "HOLLOWMERE — PROCEDURAL AUDIO GENERATOR & EXPORTER" << std::endl;
    std::cout << "Exporting all synthesized procedural audio to " << outDir << "/..." << std::endl;
    std::cout << "=========================================================" << std::endl;

    // 1. Ghost Signatures
    std::cout << "1. Synthesizing 5 Ghost Acoustic Signatures..." << std::endl;
    writeWav(outDir + "/01_ghost_weeper_sob.wav", hm::Synth2::generateWeeperSobbing(3.5f));
    writeWav(outDir + "/02_ghost_hollowman_shears.wav", hm::Synth2::generateHollowManShearsSnip());
    writeWav(outDir + "/03_ghost_hollowman_static.wav", hm::Synth2::generateHollowManStaticBurst());
    writeWav(outDir + "/04_ghost_choir_chord_concord.wav", hm::Synth2::generateChoirChord(false));
    writeWav(outDir + "/05_ghost_choir_chord_dissonant.wav", hm::Synth2::generateChoirChord(true));
    writeWav(outDir + "/06_ghost_doll_eli_giggle.wav", hm::Synth2::generateDollGiggle());
    writeWav(outDir + "/07_ghost_warden_keys.wav", hm::Synth2::generateWardenKeyJingle());
    writeWav(outDir + "/08_ghost_warden_chain_drag.wav", hm::Synth2::generateWardenChainDrag());
    writeWav(outDir + "/09_ghost_warden_roar.wav", hm::Synth2::generateWardenGutturalRoar(2.5f));

    // 2. Foley & Environmental Sounds
    std::cout << "2. Synthesizing Foley & World Audio..." << std::endl;
    writeWav(outDir + "/10_foley_footstep_wood_left.wav", hm::Synth::generateFootstep(0));
    writeWav(outDir + "/11_foley_footstep_wood_right.wav", hm::Synth::generateFootstep(1));
    writeWav(outDir + "/12_foley_door_creak_slow.wav", hm::Synth::generateDoorCreak());
    writeWav(outDir + "/13_foley_door_slam_heavy.wav", hm::Synth::generateDoorSlam());
    writeWav(outDir + "/14_foley_lantern_click.wav", hm::Synth::generateLanternClick());
    writeWav(outDir + "/15_foley_sigil_pickup_chime.wav", hm::Synth::generatePickupChime());
    writeWav(outDir + "/16_horror_ambient_drone.wav", hm::Synth::generateAmbientDrone(4.0f));
    writeWav(outDir + "/17_horror_ghost_wail.wav", hm::Synth::generateGhostWail(3.0f));
    writeWav(outDir + "/18_horror_jumpscare_stinger.wav", hm::Synth::generateJumpscareStinger());
    writeWav(outDir + "/19_body_heartbeat_panic_130bpm.wav", hm::Synth::generateHeartbeat(130.0f));
    writeWav(outDir + "/20_body_breathing_panting.wav", hm::Synth::generateBreathing(true));

    // 3. Generative Scores & Leitmotifs
    std::cout << "3. Synthesizing Generative Scores & Leitmotifs..." << std::endl;
    writeWav(outDir + "/21_score_music_box_hush_little_light.wav", hm::Synth2::generateMusicBoxMotif());
    writeWav(outDir + "/22_score_cinematic_intro_80s.wav", hm::scores::renderIntroScore());
    writeWav(outDir + "/23_score_ending1_crimson_dawn.wav", hm::scores::renderEndingScore(0));
    writeWav(outDir + "/24_score_ending2_consumed.wav", hm::scores::renderEndingScore(1));
    writeWav(outDir + "/25_score_ending3_loop.wav", hm::scores::renderEndingScore(2));
    writeWav(outDir + "/26_score_ending4_martyr.wav", hm::scores::renderEndingScore(3));
    writeWav(outDir + "/27_score_ending5_wardens_ledger.wav", hm::scores::renderEndingScore(4));

    // 4. Cutscene ambience, character motifs & typewriter ticks (no voice acting)
    std::cout << "4. Synthesizing cutscene ambience and character-recognition sounds..." << std::endl;
    namespace cs = hm::cinesfx;
    writeWav(outDir + "/28_cine_rain_bed.wav", cs::rainBed());
    writeWav(outDir + "/29_cine_fire_bed.wav", cs::fireBed());
    writeWav(outDir + "/30_cine_car_bed.wav", cs::carBed());
    writeWav(outDir + "/31_cine_thunder.wav", cs::thunder());
    writeWav(outDir + "/32_cine_gate_creak.wav", cs::gateCreak());
    writeWav(outDir + "/33_cine_gate_clang.wav", cs::gateClang());
    writeWav(outDir + "/34_cine_music_box_warped.wav", cs::musicBoxLullaby(0.8f));
    const char* names[] = {"narrator", "nora", "mae", "crane", "ada", "eli", "manor"};
    for (int s = 0; s < static_cast<int>(hm::Speaker::Count); ++s) {
        writeWav(outDir + "/motif_" + names[s] + ".wav", cs::motif(static_cast<hm::Speaker>(s)));
        writeWav(outDir + "/tick_" + names[s] + ".wav", cs::typeTick(static_cast<hm::Speaker>(s), 0));
    }

    std::cout << "=========================================================" << std::endl;
    std::cout << "SUCCESS! All procedural audio files exported to ./" << outDir << "/" << std::endl;
    std::cout << "=========================================================" << std::endl;
    return 0;
}
