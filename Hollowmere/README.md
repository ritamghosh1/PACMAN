# HOLLOWMERE

> *A single-player 3D first-person psychological horror maze built with Qt 5.15.2 and modern OpenGL 3.3 Core Profile.*

---

## 1. Overview & Story

In the winter of 1925, Nora returns to the decaying ruins of **Hollowmere Manor & Sanitarium**, seven years after a catastrophic fire killed the children under her care. Haunted by guilt, trauma, and a cryptic letter from the deceased Warden Dr. Crane, Nora must navigate five labyrinthine wings, collect five sacred Memory Sigils, confront five distinct apparitions, and uncover the terrible truth behind the orphanage's final night.

- **Protagonist**: Nora Vance (former night nurse)
- **Primary Feature**: Immersive 3D procedural acoustic atmosphere & psychological tension.
- **Strict Zero-Download Guarantee**: 100% procedural geometry, procedural normal-mapped textures, 64-voice software audio synthesis, and generative leitmotif scores. Zero downloaded 3D models, textures, or WAV files.

---

## 2. Controls & Advanced Verbs

| Key / Input | Action | In-Game Mechanics |
|---|---|---|
| **W, A, S, D** | Move | First-person movement with head bob and asymmetric limp when injured |
| **Mouse** | Look | Pitch & yaw rotation (cursor captured and hidden during play) |
| **Shift** | Sprint | Drains stamina (6s pool); louder footsteps that alert ghosts |
| **C** | Crouch | Dampens footstep volume; required to enter crawlspaces and vents |
| **E / LMB** | Interact | Open doors, pick up oil flasks, read notes, inspect puzzles |
| **F** | Lantern Toggle | Toggles kerosene spotlight. Essential for navigating darkness |
| **RMB** | Focus Lantern | Concentrates beam into high-intensity cone (burns Ada / stuns Dr. Crane) |
| **Alt** | Listen Verb | High-cut lowpass filter; visually ripples in the direction of ghost sounds |
| **Ctrl** | Blink / Close Eyes | Shuts eyes (darkens screen); Hollow Man only moves when unobserved |
| **Shift + C** | Hold Breath | Muffles breathing sound; required to evade The Choir |
| **H** | Hum / Mercy | Hold near Ada (the Weeper) to hum "Hush Little Light" and calm her |
| **M** | Minimap | Toggles the fog-of-war map (gold marker = current objective) |
| **Space / Click / E** | Story text | First press completes the typewriter line, second press continues |
| **1 – 6** | Inventory Hotkeys | Selects items (Matches, Bandages, Kerosene, Throwable Bottles) |
| **Tab** | Field Journal | Opens parchment journal with 40 lore notes, wing filters, and hints |
| **Esc** | Pause / Settings | Opens Pause menu, settings dialog, or closes journal |
| **F3 / F5 / F6 / F7 / F8** | Debug | F3 stats, F5 give all items, F6 teleport to objective, F7 open all doors, F8 summon ghosts |

---

## 3. The 5 Apparitions & Counterplay Matrix

Hollowmere features five distinct apparitions, each designed with three unique counterplay vectors: **Avoid**, **Outsmart**, and **Mercy**.

```
                   ┌──────────────────────────────────────┐
                   │          THE 5 APPARITIONS           │
                   └──────────────────┬───────────────────┘
                                      │
         ┌──────────────┬─────────────┼──────────────┬──────────────┐
         ▼              ▼             ▼              ▼              ▼
     THE WEEPER    HOLLOW MAN     THE CHOIR       THE DOLL      THE WARDEN
    (Ada - Lib)   (Pell - Cons)  (Sisters - Cel) (Eli - Nur)   (Crane - Chap)
```

1. **The Weeper (Ada Vance) — Wing 1: Library**
   - *Behavior*: Sobbing spirit wandering book stacks; agitated by loud footsteps.
   - *Avoid*: Sneak crouched; track her sobbing through door portals.
   - *Outsmart*: Focus lantern beam (`RMB`) to burn her eyes and trigger temporary retreat.
   - *Mercy*: Press `H` to rhythmically hum "Hush Little Light" to pacify her spirit.

2. **The Hollow Man (Mr. Pell) — Wing 2: Conservatory**
   - *Behavior*: Tall shadow stalker with shears; advances only when unseen or in total darkness.
   - *Avoid*: Never turn your back; keep him framed in your direct line of sight.
   - *Outsmart*: Illuminate him with lantern light; locks him completely in place.
   - *Mercy*: Close your eyes (`Ctrl`) when cornered to reveal his auditory shears cue and slip past.

3. **The Choir (Fused Sisters) — Wing 3: Boiler Cellar**
   - *Behavior*: Three fused singing heads; blind acoustic hunter tracking water ripples and noise.
   - *Avoid*: Wading through water causes loud splashes; navigate via dry raised pipes.
   - *Outsmart*: Throw glass bottles or bricks into distant corridors to lure them away.
   - *Mercy*: Stand perfectly still and hold breath (`Shift+C`) to harmonize their discordant chord.

4. **The Doll (Eli) — Wing 4: Attic Nursery**
   - *Behavior*: Porcelain doll that teleports when unobserved; ambushes from darkness.
   - *Avoid*: Listen for music-box chiming and clockwork winding gears.
   - *Outsmart*: Flash your lantern beam immediately upon hearing his giggle to stun him.
   - *Mercy*: Play the peek-a-boo mini-game (close eyes on count, open on chime).

5. **The Warden (Dr. Crane) — Wing 5: High Chapel**
   - *Behavior*: Relentless 3.6 m/s stalker; smashes through wooden doors; jangling keys.
   - *Avoid*: Break line-of-sight and hide inside iron asylum lockers or under pews.
   - *Outsmart*: Ignite flares or trigger steam valves to block corridor passageways.
   - *Mercy*: Collect all 12 memory fragments and present Dr. Crane's personal ledger at the altar.

---

## 4. Procedural Audio Engine & Acoustic Testing

The audio system operates as a real-time 64-voice stereo mixer built purely in code:
- **Woodworth ITD & ILD**: Interaural Time Difference (~0.64ms max delay) and frequency-dependent head shadow filtering.
- **8-line Householder Feedback Delay Network (FDN)**: Dynamic RT60 reverberation tailored per wing (Stone Foyer, Wood Library, Glass Conservatory, Damp Cellar, Soft Nursery, Cathedral Chapel).
- **Portal Propagation**: Sound resolves through door openings and corridors, muffling closed doors by -18 dB with a 900 Hz lowpass filter.
- **No voice acting**: story text is silent type. Characters are recognised by sound instead: each speaker has a typewriter-tick timbre and a short signature motif (Nora: felt piano, Mae: music box, Dr. Crane: tritone drone + keys, Narrator: typewriter bell, Ada: falling sob-tone, Eli: toy-piano plinks, the house: low heartbeat + creak).
- **Cutscene sound design** (`audio/CineSfx.*`): rain, wind, fire, car, thunder, gate creak/clang, music-box lullaby and more, synthesised on a worker thread at start-up. No downloaded sounds.

### Audio Testing Folder (`audio_samples/`)
The procedural audio files can be exported (`tools/audio_dump.cpp`) as sample WAV files into the dedicated directory:
```
Hollowmere/audio_samples/
├── 01_ghost_weeper_sob.wav
├── 02_ghost_hollowman_shears.wav
├── 03_ghost_hollowman_static.wav
├── 04_ghost_choir_chord_concord.wav
├── 05_ghost_choir_chord_dissonant.wav
├── 06_ghost_doll_eli_giggle.wav
├── 07_ghost_warden_keys.wav
├── 08_ghost_warden_chain_drag.wav
├── 09_ghost_warden_roar.wav
├── 10_foley_footstep_wood_left.wav
├── ...
├── 22_score_cinematic_intro_80s.wav
├── 23_score_ending1_crimson_dawn.wav
├── 28_cine_rain_bed.wav ... 34_cine_music_box_warped.wav
├── motif_<speaker>.wav / tick_<speaker>.wav   <- character-recognition sounds
└── AUDIO_CATALOG.md            <- Detailed breakdown of synthesis mechanisms & triggers
```

---

## 5. Building & Running

### Prerequisites
- macOS (Apple Silicon arm64 or Intel x86_64)
- Qt 5.15.2 (available via `/opt/anaconda3` or standard system path)
- Command Line Tools with Apple Clang (C++17)

### Build Instructions
```bash
cd /Users/ritamghosh/Desktop/Class_Notes/CG/Hollowmere
mkdir -p build && cd build
/opt/anaconda3/bin/qmake ../hollowmere.pro
make -j8
```

### Launching the Game
```bash
./hollowmere
```

### Running Unit Test Suites
```bash
# Pathfinding A* and collision tests
./tests/test_pathfind

# Save/Load serialization tests
./tests/test_save

# Story choices and ending resolution tests
./tests/test_ending

# Audio DSP and spatial panning tests
./tests/test_audio
```

---

## 6. Project Architecture

```
Hollowmere/
├── src/
│   ├── ai/          # A* NavGrid pathfinding, Sensory cone raycasting, Director pacing curve
│   ├── app/         # MainWindow, GameState, Difficulty modes, DebugConsole
│   ├── audio/       # AudioEngine 64-voice mixer, Synth, Spatial ITD/ILD, FDN Reverb, VoiceFx, Scores
│   ├── core/        # Mathx library (Vec2/3/4, Mat4, AABB, Ray), Clock (120Hz sim), Input, PRNG
│   ├── entities/    # Player, Lantern, Ghost base class, Weeper, HollowMan, Choir, Doll, Warden
│   ├── gameplay/    # Inventory, Health, Sanity, Hallucinations, Verbs, Mercy mini-games, Death
│   ├── gfx/         # GLView, Shader, MeshBuilder, Camera, Renderer, PostFx, Shadows, Decals
│   ├── puzzle/      # SigilAltar, CipherPuzzle, MirrorPuzzle, ValvePuzzle, MusicBoxPuzzle
│   ├── save/        # SaveGame JSON serialization system
│   ├── story/       # Choices, Notes database, 12 Memory Fragments, Journal, Cutscenes, Subtitles
│   ├── ui/          # HUD, MenuOverlay, PauseMenu, JournalWidget, InventoryWidget, RhythmWidget
│   └── world/       # MapGen, Collision, Doors, Pickups, Props, PropsGore, SafeRooms, Wings
├── shaders/         # world.vert, world.frag, postfx.frag, ghost.frag, shadow.vert
├── tests/           # Unit test executables
└── audio_samples/   # Standalone procedural WAV exports and playback catalog
```

---
*Created for Computer Graphics Systems Laboratory — Standalone 3D Engine Architecture.*


---

## 6. Story Presentation & Gameplay Flow (v2)

- **Typewriter text everywhere.** Cutscenes, notes, hints and the first-steps briefing all use one click-to-continue typewriter box (`story/Dialogue.*`).
- **Animated cutscenes** (`story/Cinematic.*`, `story/CineArt.*`, `story/Cutscenes*.cpp`): a six-shot intro (the locked door in 1998, the rain-streaked window, the music box, the drive, the gate, the title) and five ending epilogues + credits. Preview any of them without OpenGL or audio: `qmake tools/cine_preview.pro && make && ./cine_preview intro out/`.
- **A clear quest.** Four wings, four memory sigils, one key per wing, then the chapel altar. The HUD shows the current objective, a compass arrow, an on-screen marker and a fog-of-war minimap; the objective item also glows in the world.
- **Colour-coded wings.** Library (timber), Conservatory (green), Boiler Cellar (iron blue), Nursery (pink), Chapel (blood stone). Each shows a typed location banner when you enter.
- **Safe rooms.** The foyer and the candle-lit entrance of every wing: ghosts cannot hurt you there.
- Fixed in this version: edge-triggered input (E / F / clicks) fired twice per frame so doors and the lantern appeared not to work; doors now fit their doorways and swing; ghosts used a wrong forward vector; navigation covered only a fraction of the map; Continue/Load rebuilt the level without a GL context.

### v2.1 changes
- **Performance:** the project now always builds an optimised binary (`CONFIG += release`; on macOS a plain `make` used to build an unoptimised debug binary). The 3D scene no longer needs window MSAA, the HUD is rasterised at logical resolution, fonts are cached and the minimap is drawn from a cached image.
- **Graphics quality now does something** (Settings): Low = half resolution, no shadows, 4 lights; Medium (default) = 3/4 resolution, soft shadows, 8 lights; High = native resolution, 12 lights. It applies immediately.
- **Humming to Ada** is a real mini-game: stand within earshot, hold H, then click or press Space when the marker is in the green. The panel only shows while it can work.
- **Story clarity:** animated memory flashbacks after each sigil explain who the ghost was; the objective panel says why each item matters; the pause screen shows "The story so far".
- **Demo mode:** the old Credits slot is now **STORY & ENDINGS**: a readable summary of the story and how it plays, all five endings with how to reach each, and buttons to watch the intro and any ending. Endings finish on a summary of all five instead of credits.
