# HOLLOWMERE — PROCEDURAL AUDIO AUDIT & PLAYBACK GUIDE

All 31 audio files below were procedurally synthesized in real time using additive Fourier synthesis, physical room resonance modeling, Woodworth interaural time difference (ITD), head-shadow lowpass filters (ILD), formant vocal tracts, and 8-line Householder Feedback Delay Networks (FDN). No external audio files or recorded samples were used.

---

## 1. Ghost Acoustic Signatures (Phase 3 & Phase 7)

| Filename | Duration | Synthesis Mechanism | In-Game Trigger Location & Context |
|---|---|---|---|
| `01_ghost_weeper_sob.wav` | 3.5s | Convulsive breath intakes (bandpass noise) + pitch-sliding vocal formant lamentation (380-520 Hz) | **Wing 1 (Library)**: Emitted by *The Weeper (Ada)* every 3.5s when within 14m of the player. Softens when Nora hums "Hush Little Light". |
| `02_ghost_hollowman_shears.wav` | 0.12s | High-frequency metal transient clash (3.8 kHz resonance) + scissor snap impulse | **Wing 2 (Conservatory)**: Emitted by *The Hollow Man (Mr. Pell)* when stalking unobserved in darkness or behind glass domes. |
| `03_ghost_hollowman_static.wav` | 0.35s | Swept white noise burst + electromagnetic tape hum | **Wing 2 (Conservatory)**: Plays when player turns to look directly at The Hollow Man, signaling his freezing reaction. |
| `04_ghost_choir_chord_concord.wav` | 4.0s | 3 parallel vocal tract formants (D3 146.8 Hz, F3 174.6 Hz, A3 220.0 Hz) | **Wing 3 (Boiler Cellar)**: Harmonious acoustic signature of *The Choir* when pacified by holding breath and standing still. |
| `05_ghost_choir_chord_dissonant.wav` | 4.0s | Dissonant tritone chord (D3 146.8 Hz, Eb3 155.6 Hz, G#3 207.7 Hz) + flutter | **Wing 3 (Boiler Cellar)**: High-agitation hunting cry of The Choir when noise (splash/running) is detected on acoustic bus. |
| `06_ghost_doll_eli_giggle.wav` | 1.4s | Inharmonic pitch-rising chirps (780→1400 Hz) + clockwork gear clicks | **Wing 4 (Nursery/Attic)**: Scripted & dynamic ambush cue for *The Doll (Eli)* before teleporting behind the player. |
| `07_ghost_warden_keys.wav` | 0.8s | Metallic iron bar impacts (2.4 kHz & 3.2 kHz) with erratic multi-tap scatter | **Wing 5 (Chapel)**: Stalker approach cue of *The Warden (Dr. Crane)*, audible through doorways and walls from 28m away. |
| `08_ghost_warden_chain_drag.wav` | 2.0s | Heavy metallic friction scrape + low-frequency asphalt grind | **Wing 5 (Chapel)**: Relentless pursuit indicator when Dr. Crane speeds up to 3.6 m/s during the finale chase. |
| `09_ghost_warden_roar.wav` | 2.5s | Pitch-descending distorted predator formant (180→65 Hz) + sub-bass chest thump | **Wing 5 (Chapel)**: Attack scream of The Warden upon closing distance or smashing down a wooden barricade door. |

---

## 2. Foley, World & Physiological Audio (Phase 2 & Phase 7)

| Filename | Duration | Synthesis Mechanism | In-Game Trigger Location & Context |
|---|---|---|---|
| `10_foley_footstep_wood_left.wav` | 0.22s | Low wood thud (120 Hz) + floorboard creak transient | Left footstep on wooden parquet floors throughout the Foyer, Library, and Nursery. |
| `11_foley_footstep_wood_right.wav` | 0.22s | Slightly detuned wood thud + shoe sole friction | Right footstep; features asymmetrical limp timing when player is in `Critical` health state. |
| `12_foley_door_creak_slow.wav` | 1.8s | Frequency-modulated squeak (850→620 Hz) + wood stress friction | Played when opening or pushing unlocked manor doors. |
| `13_foley_door_slam_heavy.wav` | 0.65s | High-energy broadband impact transient + 45 Hz sub rumble decay | Triggered by Director soft-scare events and intro gate closure. |
| `14_foley_lantern_click.wav` | 0.08s | Brass latch mechanical click + subtle kerosene wick mantle hiss | Toggled when turning lantern on/off (`F`) or focusing the beam (`RMB`). |
| `15_foley_sigil_pickup_chime.wav` | 0.55s | Ethereal bell resonance (A5 880 Hz + harmonics) with long reverb tail | Plays upon picking up any of the 5 story Sigils or brass keys. |
| `16_horror_ambient_drone.wav` | 4.0s | Dual detuned sine wave infrasound (42 Hz & 44 Hz) with slow beating | Continuous atmospheric dread bed beneath the manor soundscape. |
| `17_horror_ghost_wail.wav` | 3.0s | Swept resonant bandpass noise filter through Householder FDN reverb | Distant apparition wail heard through portal propagation corridors. |
| `18_horror_jumpscare_stinger.wav` | 1.4s | Sudden cluster chord (diminished 7th + minor 2nd) with instant 2ms attack | High-threat jump cue played on ambush or lethal ghost strike. |
| `19_body_heartbeat_panic_130bpm.wav` | 0.45s | Dual sub-bass thud (lub-dub at 55 Hz and 48 Hz) with visceral distortion | Plays rhythmically when Director fear meter > 70% or sanity is critical. |
| `20_body_breathing_panting.wav` | 1.0s | Airflow turbulence noise through dual lung formant filter | Exhaustion audio when stamina bar is fully depleted from sprinting. |

---

## 3. Generative Scores & Leitmotifs (Phase 2, Phase 5, Phase 7)

| Filename | Duration | Synthesis Mechanism | In-Game Trigger Location & Context |
|---|---|---|---|
| `21_score_music_box_hush_little_light.wav` | 7.8s | Inharmonic chime tines (D minor: D4, F4, A4, G4, F4, E4, D4) with bell decay | The core emotional leitmotif; plays in the Nursery music-box puzzle, Ada's lullaby mercy mini-game, and memory flashbacks. |
| `22_score_cinematic_intro_80s.wav` | 80.0s | 80s cinematic orchestral progression: pedal tone, cello drone, rising string crescendo, gate impact, silence tail | Complete musical score for the 2.5D paper-cut puppet Intro Movie (`T-071`). |
| `23_score_ending1_crimson_dawn.wav` | 16.0s | Warm D Major brass & felt piano resolution | **Ending 1 (Release)**: Played when all 3 child spirits are spared and dawn arrives. |
| `24_score_ending2_consumed.wav` | 13.0s | Low descending cello drone dissolving into hollow wind | **Ending 2 (Consumed)**: Played when Nora succumbs to guilt and is trapped in darkness. |
| `25_score_ending3_loop.wav` | 13.0s | Repeating cyclical felt piano ostinato | **Ending 3 (Loop)**: Played when Nora flees through the gate only to awaken in the Foyer. |
| `26_score_ending4_martyr.wav` | 14.0s | Somber vocal choir chorale + fire rumble crackle | **Ending 4 (Sacred Ashes)**: Played when the manor burns to cinder to contain the evil. |
| `27_score_ending5_wardens_ledger.wav` | 13.0s | Bittersweet music-box harmonized with church organ | **Secret Ending 5 (The Warden's Ledger)**: Unlocked by collecting all 12 memory fragments. |

---

## 4. Cutscene Ambience & Character Recognition (replaces the old voice pipeline)

The game has **no voice acting**. Story text is typed on screen and each character is identified by sound:

| Sound | Description | Used for |
|---|---|---|
| `tick_<speaker>` | Per-character typewriter tick. Narrator: typewriter strike. Nora: soft felt tap. Mae: glass ping. Crane: cold metal clack. Ada: airy breath. Eli: wooden block. House: dull knock. | Played as each letter appears |
| `motif_<speaker>` | ~1.5 s signature phrase (Nora: D4-A4 felt piano; Mae: music-box E-G-B with a flat last tine; Crane: tritone drone + key jangle; Narrator: typewriter bell; Ada: falling wavering tone; Eli: toy plinks + block clacks; House: heartbeat + creak) | Played whenever a different character starts speaking |
| `28_cine_rain_bed` ... `34_cine_music_box_warped` | Loop-safe rain, fire and car beds, thunder, gate creak/clang, warped lullaby | Intro and ending cutscenes |

Regenerate with `tools/audio_dump.cpp`.
