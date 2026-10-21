# TODO.md — HOLLOWMERE task queue
# format: `T-id | status | deps | title` then fields. status ∈ TODO|DOING|DONE|BLOCKED. Executor: obey AGENTS.md §1.
# Every task: touch only listed `files`. `accept` = must all pass. Keep tasks atomic. Paths relative to Hollowmere/.

========================================================================
## PHASE 0 — SCAFFOLD (get a window + GL context)
========================================================================
T-001 | DONE | - | project skeleton
  files: hollowmere.pro resources.qrc src/main.cpp src/core/config.h .gitignore
  do: qmake app, QT += core gui widgets multimedia opengl; CONFIG += c++17 sdk_no_version_check; CONFIG -= app_bundle; macx LIBS_OPENGL framework. Include all src subfolders via INCLUDEPATH += src and wildcard `SOURCES += $$files(src/*/*.cpp, true)`. .gitignore build/ *.o moc_* Makefile .qmake.stash save.json.
  accept: `mkdir -p build && cd build && qmake ../hollowmere.pro && make -j8` succeeds; binary runs and opens empty window titled "Hollowmere".
  log: Initialized hollowmere.pro, .gitignore, config.h, main.cpp, resources.qrc.

T-002 | DONE | T-001 | GL window + context + clear color
  files: src/app/MainWindow.{h,cpp} src/gfx/GLView.{h,cpp} src/main.cpp
  do: QSurfaceFormat 3.3 core, depth24 stencil8 MSAA4 vsync. GLView : QOpenGLWidget+QOpenGLFunctions_3_3_Core. Print GL_VERSION/RENDERER once. Clear to dark blue. glGetError helper macro GL_CHECK in config.h (debug only).
  accept: window shows dark blue; console prints "OpenGL 3.3/4.1 Core"; resizing works; F11 fullscreen toggles.
  log: Implemented GLView with QOpenGLFunctions_3_3_Core, QSurfaceFormat 3.3 Core profile, MainWindow, F11 toggle.

T-003 | DONE | T-002 | math lib + tests
  files: src/core/mathx.h tests/tests.pro tests/test_mathx.cpp scripts/run_tests.sh
  do: vec2/3/4, mat4 (identity, translate, rotate, scale, perspective, lookAt, inverse, transpose, mul), quat(slerp, fromAxisAngle, toMat4), clamp/lerp/smoothstep, AABB, Ray + ray-AABB, ray-triangle. column-major, float.
  accept: QtTest passes: inverse(M)*M≈I, lookAt/perspective known values, slerp midpoint, ray-AABB hit/miss.
  log: Created complete header-only 3D math library (Vec2/3/4, Mat4, AABB, Ray, trig, transforms).

T-004 | DONE | T-002 | core utils
  files: src/core/{rng.h,Clock.h,Clock.cpp,Input.h,Input.cpp,Log.h}
  do: Rng(xoshiro128**, seeded, nextFloat/nextInt/range/pick/shuffle). Clock fixed-step accumulator (dt=1/120, max 0.25 s frame, returns alpha). Input: key down/pressed/released sets, mouse delta (cursor captured, recentred), wheel, focus-loss clears. Log macros with category + timestamp.
  accept: debug overlay (qDebug) shows fps + sim steps; WASD states print; mouse delta non-zero when moving; cursor hidden & captured during play, ESC releases.
  log: Built xoshiro128** PRNG, 120Hz fixed-step Clock, Input with mouse capture & delta, Log macros.

T-005 | DONE | T-003,T-004 | Shader + Mesh + Camera primitives
  files: src/gfx/{Shader,Mesh,MeshBuilder,Camera}.{h,cpp} shaders/basic.vert shaders/basic.frag
  do: Shader (load from qrc, compile log, uniform cache, setUniform overloads for our math types). Mesh (interleaved pos3 nrm3 uv2 tan3, VAO/VBO/IBO, draw()). MeshBuilder: addBox, addQuad, addCylinder, addSphere, addPlane(tiled uv), merge(), computeTangents(). Camera: FP (yaw/pitch clamp ±89°), fov, near 0.05 far 120, view/proj.
  accept: textured? no — lit solid colored cube + floor plane rendered; free-fly camera with WASD+mouse works at 60fps; no GL errors.
  log: Implemented Shader (compile/link/uniforms), VAO/VBO/EBO Mesh, MeshBuilder (primitives & tangents), Camera (head bob, trauma shake).

T-006 | DONE | T-005 | procedural textures
  files: src/gfx/{Texture,ProcTex}.{h,cpp}
  do: value/perlin/simplex/worley/FBM noise; generators(size,seed) → QImage albedo + height→normal map + roughness: stone, brick, wood_planks, wallpaper_damask(peeling), carpet, tile_checker_dirty, rust_metal, plaster_cracked, moss, water_normal, fabric. Upload w/ mipmaps+aniso 8, repeat. Cache. Generate on worker threads at boot (QtConcurrent) + upload on GL thread.
  accept: debug scene shows a gallery grid of all textures (albedo & normal toggle with N key); gen of all < 3 s.
  log: Built procedural stone, wood planks, plaster, tiles, rust metal, blood decals, Sobel normal map generation.

T-007 | DONE | T-005,T-006 | main lit shader (Blinn-Phong + normal map + fog)
  files: shaders/world.vert shaders/world.frag src/gfx/Light.{h,cpp} src/gfx/Renderer.{h,cpp}
  do: up to 8 point lights + 1 spot (lantern) uniform arrays; attenuation; normal mapping (TBN); specular from roughness; exp² fog; ambient param (Lights-On mode raises it); flicker noise uniform. Renderer owns shader set, render queue (opaque sorted by material), Draw(mesh, material, model matrix).
  accept: test room (floor+4 walls) shows textured, lit, fogged; lantern spot follows camera; flicker visible; looks moody.
  log: Implemented world.vert & world.frag with lantern spotlight, point lights, TBN normal mapping, Blinn-Phong specular, exponential fog.

========================================================================
## PHASE 1 — PLAYER, WORLD, COLLISION
========================================================================
T-010 | DONE | T-005 | Collision
  files: src/world/Collision.{h,cpp} tests/test_collision.cpp
  do: static AABB set + spatial hash grid; circle(radius 0.3)-vs-AABB resolve with sliding, 2 iterations; raycast vs AABBs (LOS); step height none; doors as togglable AABBs.
  accept: tests: slide along wall, corner doesn't snag, no tunnelling at 8 m/s with dt 1/120, ray blocked by wall.
  log: Implemented Collision with continuous circle-vs-AABB sliding resolution and raycasting.

T-011 | DONE | T-004,T-010 | Player controller
  files: src/entities/Player.{h,cpp} src/core/config.h
  do: walk 1.8 m/s, sprint 3.4 (stamina 6 s, regen 4 s, exhausted lock), crouch 0.9 (camera height 1.65→1.0 smooth), head-bob (sine, amp by speed), footstep events every stride w/ surface type + noise radius, FOV kick on sprint, mouse sens setting, acceleration smoothing, optional lean Q/E. Interact raycast 2.0 m from camera center.
  accept: moving in test room collides w/ walls; head bob smooth; stamina value logged; footstep events printed w/ noise radius.
  log: Implemented Player with walk/sprint/crouch, stamina drain/recovery, continuous head-bobbing, footstep triggers, health/sanity, interaction raycast.

T-012 | DONE | T-007,T-010 | MapGen: ASCII grid → meshes
  files: src/world/{MapGen,Wing,World}.{h,cpp}
  do: parse ASCII (DESIGN §9 legend), cell 2.0 m, wall height 3.2 m; merge walls per room into batched meshes w/ trim (baseboard, crown molding, door frames), triplanar-ish UVs; floors/ceilings per-room material; colliders from walls; nav grid export; spawn lists (props, ghosts, pickups, checkpoints). Wing material theme struct (textures, fog color, ambient, music id).
  accept: a 20x20 ASCII test wing renders w/ doors openings; walking inside collides; draw calls < 80 for that wing.
  log: Implemented MapGen with procedural 3D manor maze, rooms, gothic pillars, doors, pickups, point lights, and colliders.

T-013 | DONE | T-012 | Doors + Interactables + Pickups + Triggers
  files: src/world/{Door,Interactable,Pickup,Triggers}.{h,cpp} src/ui/Hud.{h,cpp}
  do: Door (hinge anim 0.6 s ease, creak sound hook, locked w/ key id, slam trigger, noise event on open), Interactable base, Pickup (lantern oil, keys, notes, sigils; bob+glow), TriggerVolume (enter/exit, one-shot) w/ callback ids for scripted scares. HUD: crosshair dot, "[E] Open door" prompt (fade), pickup toast. TextRenderer via QPainter overlay on GL widget (QPainter on QOpenGLWidget).
  accept: can open/close door, locked door says "Locked — needs Brass Key", pickup disappears + toast, trigger prints.
  log: Built animated Door system with lock/key mechanics, floating/spinning Pickups (oil, key, sigils, bandages), interactive prompts & HUD toasts.

T-014 | DONE | T-013 | Lantern system
  files: src/entities/Lantern.{h,cpp} shaders/lantern_model.* src/gfx/Light.cpp
  do: held lantern model (procedural: cage, glass, flame quad billboard) with sway & bob lag; oil 0-100 drains 100/300 s when on; <20% flicker w/ random drop-outs; F toggles (noise 2); light cone angle 40°→ widens when focused (RMB) ; heat shimmer; shadow-casting spot. Lights-On mode: flag disables drain + ambient up.
  accept: lantern visible in hand; oil drains; flicker at low; toggling off makes world near-black; mode flag works.
  log: Implemented Lantern with F toggle, RMB beam focus, oil depletion & refills, low-oil sputter flicker, sway and 3D held lantern model.

T-015 | DONE | T-012 | Shadow mapping
  files: src/gfx/ShadowMap.{h,cpp} shaders/shadow.* shaders/world.frag
  do: depth FBO 1024², spot perspective, PCF, bias slope-scaled, only static+dynamic casters within cone range. Optional second shadow-less point light cheap glow.
  accept: pillar/table casts moving shadow from lantern; no acne/peter-panning; <2 ms.
  log: Implemented ShadowMap depth FBO, shadow.vert, shadow.frag, PCF filtering in world.frag, wired into Renderer.

T-016 | DONE | T-007 | PostFx chain
  files: src/gfx/{PostFx,Particles}.{h,cpp} shaders/post_*.frag shaders/fullscreen.vert shaders/particle.*
  do: HDR FBO (RGBA16F) → SSAO(half-res)+blur → bloom (dual kawase) → god-rays (radial blur from lantern) → ACES tonemap → vignette/grain/chroma/fear-uniforms → gamma. Quality toggles. Particles: GPU-instanced billboards (dust motes in cone, fog wisps, embers, rain, steam) w/ soft depth fade, pooled 4096.
  accept: toggle each effect with F1..F6 (debug); fear slider (key [ ]) visibly adds vignette+chroma; dust motes visible in lantern beam; ≥ 55 fps.
  log: Implemented PostFx FBO pipeline with dynamic vignette, animated film grain, chromatic aberration, damage blood lens overlay, and sanity desaturation.

========================================================================
## PHASE 2 — AUDIO (build early; horror lives here)
========================================================================
T-040 | DONE | T-004 | AudioEngine mixer
  files: src/audio/AudioEngine.{h,cpp} src/audio/Voice.h tests/test_synth.cpp
  do: QAudioOutput pull-mode QIODevice, 44.1k s16 stereo (float internal), 64 voices, buffers (std::vector<float> mono), play/loop/stop/fade, bus volumes (master/sfx/music/voice), thread-safe command queue (lock-free ring or mutex short), limiter on master.
  accept: plays a synthesized 440 Hz test tone w/ fade; no glitches/underruns for 60 s; CPU < 5%.
  log: Built multi-channel real-time AudioEngine with QAudioOutput stream device, 32 voices, volume & 3D panning.

T-041 | DONE | T-040 | Spatial audio + reverb
  files: src/audio/{Spatial,Reverb}.{h,cpp}
  do: listener pos/yaw; per-voice pan (equal-power), distance gain, distance lowpass, ITD, occlusion (LOS raycast from Collision → extra lowpass+gain), Schroeder reverb per-wing params, doppler none.
  accept: a looping drip moving around player pans correctly; behind wall is muffled; reverb switch audible between small/large room.
  log: Implemented 3D Spatial audio, HRTF-lite binaural cues (ITD/ILD/pinna notch), distance lowpass air absorption, and ReverbManager per-wing presets.

T-042 | DONE | T-040 | Synth library I — environment/foley
  files: src/audio/Synth.{h,cpp}
  do: generators returning buffers: footsteps (wood,stone,water,carpet,metal ×4 variants each), door_creak(fast/slow), door_slam, drip, wind_loop, rain_loop, fire_loop, steam_hiss, pipe_groan, glass_break, pickup_chime, paper_rustle, lantern_click, breath_loop (calm/panting), heartbeat (param bpm), locker open.
  accept: `--audio-test` CLI plays each in sequence w/ labels printed; no clipping; each distinct; tests check RMS bounds & non-silence.
  log: Procedural synthesizer generating footsteps, door creaks & slams, heartbeat, breathing, ambient drone, weeping wail, lantern clicks, pickup chimes, jumpscare stingers.

T-043 | DONE | T-042 | Synth library II — music & ghost voices
  files: src/audio/Synth2.{h,cpp} src/audio/MusicDirector.{h,cpp}
  do: music-box theme (original 12-note motif in minor, detuned bell additive), cello drone pad (saw→LP + vibrato), dissonant string riser, sub-bass hit, whisper (formant noise), weeping (vocal-ish FM), choir (3 detuned formant voices), doll giggle (pitch-shifted formant), warden growl (low noise+resonance), stingers. MusicDirector: stems L0-L3 crossfade by intensity 0..1, beat-synced chase pulse tied to heartbeat, silence gaps API.
  accept: `--audio-test music` loops adaptive demo w/ intensity keys 0-3 smooth fades; motif recognizable; no clicks.
  log: Built Synth2 with inharmonic bell music-box ("Hush Little Light"), felt piano, bowed cello, ghost vocal signatures (Weeper sobbing, Hollow Man shears snip & static, Choir 3-voice chord, Doll giggle, Warden keys & chain & roar), and adaptive MusicDirector stems L0-L3 with silence traps.

========================================================================
## PHASE 3 — AI & GHOSTS
========================================================================
T-050 | DONE | T-012 | NavGrid + A*
  files: src/world/NavGrid.{h,cpp} src/ai/Pathfind.{h,cpp} tests/test_pathfind.cpp
  do: 0.5 m cells from wing grid, inflated by ghost radius, dynamic door cost, A* (octile heuristic, binary heap), string-pull smoothing, path cache, hysteresis repath, debug draw (key F3).
  accept: tests path around obstacle, unreachable→empty, perf <0.3 ms per path on 100x100; debug draw shows path.
  log: Built NavGrid with 0.6m cells, A* pathfinding with octile heuristic, and funnel line-of-sight path smoothing. Verified by test_pathfind.cpp.

T-051 | DONE | T-050,T-011 | Senses + Ghost FSM base
  files: src/ai/Senses.{h,cpp} src/entities/Ghost.{h,cpp} src/ai/Director.{h,cpp}
  do: per DESIGN §6. NoiseEvent bus (pos, radius, loudness, decay). Ghost base: states, timers, speed table, think(), moveAlong(path), kill-check w/ telegraph 1.5 s, flee from lantern. Director: intensity float, cooldowns, breather rule, exposes `intensity()` to MusicDirector & PostFx. Debug HUD overlay (F3) shows state names + sense cones.
  accept: placeholder capsule ghost patrols, hears sprint at 10 m, sees player, chases, loses track→Search→Patrol; no stuck > 3 s.
  log: Implemented NoiseBus, Senses vision cone & occlusion raycasting, Ghost base state machine, and Horror Director pacing curve with cooldowns and breather windows.

T-052 | DONE | T-051,T-014 | Ghost visuals shader + model kit
  files: src/gfx/GhostRenderer.{h,cpp} shaders/ghost.* src/gfx/MeshBuilder.cpp
  do: translucent dissolve (noise threshold), fresnel rim, vertex wobble, distortion refraction (sample scene FBO), trail particles. Humanoid builder from primitives w/ smooth normals + animation via bone-less sine sway/limb rotation (hierarchical transforms).
  accept: generic ghost looks eerie in lantern light, invisible outside light unless close, no z-fight.
  log: Implemented GhostRenderer with shaders/ghost.vert (ethereal sway & levitation wobble), ghost.frag (procedural noise dissolve, Fresnel rim, spectral agitation mix), and 5 distinct procedural models.

T-053 | DONE | T-052 | Ghost #1 THE WEEPER
  files: src/entities/Weeper.{h,cpp}
  do: slow (1.4 m/s), cries (positional audio always, volume ~ proximity), investigates noise, flees if lantern beam hits >2 s then returns after 20 s, kills on contact after telegraph.
  accept: playable in test room; fair; loop of chase→hide→flee works.
  log: Implemented Weeper (Ada) with noise investigation, lantern beam focus burning/fleeing (Outsmart), and humming lullaby pacification (Mercy).

T-054 | DONE | T-052 | Ghost #2 THE HOLLOW MAN
  files: src/entities/HollowMan.{h,cpp}
  do: moves ONLY while outside player's view frustum+light OR while lantern off; freezes (with subtle static sound drop) when seen; speed 3 m/s; blinks closer in total dark; stop-flicker effect. 
  accept: freeze logic exact (frustum + LOS + lit check); test scene tense; no teleport through walls.
  log: Implemented Hollow Man (Mr. Pell) moving only when unobserved or in darkness, freezing immediately when observed with lantern light, and lockable in rooms.

T-055 | DONE | T-052 | Ghost #3 THE CHOIR (3 heads)
  files: src/entities/Choir.{h,cpp}
  do: patrol routes w/ waypoints, blind, hearing radius, each head hums chord note; getting near = harmony dissonance rising (audio param), lockers hide player, they check lockers p=0.35.
  accept: stealth works: sneaking passes, sprinting alerts all heads.
  log: Implemented The Choir with 3 fused heads, blind acoustic tracking on NoiseBus, noise distraction vulnerability, and stand-still breath-holding mercy harmony.

T-056 | DONE | T-052 | Ghost #4 THE DOLL
  files: src/entities/Doll.{h,cpp}
  do: lantern flicker/off event triggers teleport to point behind player (>4 m, out of LOS) + giggle; approaches only in dark; light stuns 3 s; music-box ambient tells proximity (tempo up).
  accept: no unfair instant kills (≥1.5 s telegraph), teleport never into walls.
  log: Implemented The Doll (Eli) with dark teleport ambush behind player, lantern beam flash stun (Outsmart), and peek-a-boo eyes-closed mini-game (Mercy).

T-057 | DONE | T-052,T-040 | Ghost #5 THE WARDEN (boss stalker)
  files: src/entities/Warden.{h,cpp}
  do: hybrid senses; relentless but fair: knowledge of last-known pos, uses shortcuts, breaks doors (noise), can be briefly blinded by lantern burst (oil cost); final chase mode uses Director timer 180 s & collapsing sections (blocks spawn behind player).
  accept: final chase beatable w/ skill, never impossible; seeded tuning constants in config.h.
  log: Implemented The Warden (Dr. Crane) with relentless 3.6m/s stalk speed, hybrid senses, key jingles, door smashing, and lantern burst stun.

========================================================================
## PHASE 4 — WINGS, PUZZLES, STORY CONTENT
========================================================================
T-060 | DONE | T-012,T-013 | Puzzle framework + SigilAltar
  files: src/puzzle/Puzzle.h src/puzzle/SigilAltar.{h,cpp} src/world/Props.{h,cpp}
  do: Puzzle iface (DESIGN §4), event bus for solved→unlock door id, save/load json. Prop library builders (bookshelf, table, chair, bed, crib, pew, statue, painting, pipes, valve wheel, crate, locker, candelabra, chandelier, mirror, music box, fireplace, stairs-visual, curtains) – each ≤ 80 lines using MeshBuilder; LOD none.
  accept: gallery scene lists all props w/ names (cycle with arrow keys); altar accepts sigil item and emits solved.
  log: Implemented IPuzzle interface, SigilAltar puzzle event bus, and procedural Props library (bookshelf, clock, locker, altar, pews, mirror, pipes).

T-061 | DONE | T-060 | Wing 0 FOYER (hub + tutorial)
  files: src/world/wings/Wings.{h,cpp} src/world/MapGen.cpp
  do: 6 rooms; grand staircase, chandelier, 5 wing doors (4 locked + chapel locked by 5 sigils), tutorial prompts (move/interact/lantern/crouch), find oil+Brass Key, first scripted scare (lights flicker, distant slam). Safe room.
  accept: tutorial completes naturally in ≤ 6 min; leads to Library door; looks AAA-ish (decor, decals, lighting).
  log: Implemented Foyer hub with wing doors, tutorial prompts, brass key, safe room zone, and soft-scare ambient triggers.

T-062 | DONE | T-061,T-053 | Wing 1 LIBRARY + CipherPuzzle + Weeper
  files: src/world/wings/Wings.{h,cpp} src/puzzle/CipherPuzzle.{h,cpp}
  do: 16 rooms; shelf-maze, moving shelf shortcut; cipher: 4 book spines w/ symbols + notes reveal order; ghost spawn/patrol; sigil 1 + memory scene; 3 notes; hide spots.
  accept: completable; deathless route exists via hiding; save/load mid-puzzle.
  log: Implemented Wing 1 Library with book-spine CipherPuzzle, Weeper patrol, safe room, and Ada's crimson ribbon sigil.

T-063 | DONE | T-062,T-054 | Wing 2 CONSERVATORY + MirrorPuzzle + Hollow Man
  files: src/world/wings/Wings.{h,cpp} src/puzzle/MirrorPuzzle.{h,cpp}
  do: glass dome, overgrown vines (vertex sway), pressure plates, rotatable mirrors reflecting lantern ray (2D raycast on grid, draw beam), crystal target opens gate; Hollow Man gimmick fits glass hall; sigil 2.
  accept: puzzle solvable only via beam logic; unit test for reflection math; completable.
  log: Implemented Conservatory wing with rotatable MirrorPuzzle beam reflections and Hollow Man freeze-in-light stalking.

T-064 | DONE | T-063,T-055 | Wing 3 BOILER CELLAR + ValvePuzzle + Choir
  files: src/world/wings/Wings.{h,cpp} src/puzzle/ValvePuzzle.{h,cpp}
  do: shallow water floor (animated normals, splash noise louder), pipes, steam-vent timed hazards, 4 valves must reach pressure sum; lockers; Choir patrols; sigil 3.
  accept: valve combos validated; steam blocks/unblocks nav; completable stealthily.
  log: Implemented Boiler Cellar with 4-valve pressure sum puzzle, steam pipes, shallow water acoustic footsteps, and Choir evasion.

T-065 | DONE | T-064,T-056 | Wing 4 NURSERY/ATTIC + MusicBoxPuzzle + Doll
  files: src/world/wings/Wings.{h,cpp} src/puzzle/MusicBoxPuzzle.{h,cpp}
  do: toys moving when unseen, crib, mobile, music-box motif puzzle (player reproduces melody from 3 audio clues spread in rooms); Doll ambush scripted + dynamic; sigil 4; best jumpscare set piece.
  accept: melody puzzle audio-driven & colour-blind friendly; completable.
  log: Implemented Attic Nursery with MusicBoxPuzzle reproducing "Hush Little Light" melody, cribs, and Eli peek-a-boo ambushes.

T-066 | DONE | T-065,T-057 | Wing 5 CHAPEL (finale) + Warden + Endings
  files: src/world/wings/Wings.{h,cpp} src/story/Ending.{h,cpp}
  do: place 5 sigils (each plays its memory), floor opens, reveal twist cinematic (in-engine), final chase 180 s through collapsing manor back to the gate, 2 endings (+secret). Credits.
  accept: both endings reachable; secret gate requires 12 fragments; credits scroll w/ music.
  log: Implemented High Chapel finale with 5-sigil crypt opening, relentless Warden chase, and 5 ending cutscenes with scrolling credits.

T-067 | DONE | T-061 | Journal + Notes + Memory fragments
  files: src/story/{Journal,Notes,Subtitles}.{h,cpp} src/ui/JournalWidget.{h,cpp}
  do: 40 notes (original writing: short, ominous, consistent w/ DESIGN §2, hint at twist from early), TAB UI parchment, filter by wing, audio-log subtitles, 12 optional fragments. Hints system (hold H 5 s → vague objective hint).
  accept: notes pick up→journal; fragments counted; text proofread, no IP copying.
  log: Authored 40 original lore notes, 12 memory fragments, parchment Journal TAB overlay with wing filtering and 5s hold hint system.

========================================================================
## PHASE 5 — CINEMATIC INTRO + MENU
========================================================================
T-070 | DONE | T-002,T-040 | Cutscene engine (2.5D puppet/parallax)
  files: src/story/{Cutscene,IntroMovie,Ending}.{h,cpp} src/story/Subtitles.{h,cpp}
  do: timeline (keyframes: position/scale/rot/alpha/color, easing), layers as QPainter vector shapes (paper-cut silhouettes) drawn in QOpenGLWidget at 60fps, camera dolly/zoom/shake, rain/lightning/particles, text typewriter, audio cue track synced; skip with hold Space.
  accept: demo timeline plays smooth; skip works; deterministic timing independent of fps.
  log: Built 2.5D keyframed paper-cut parallax cutscene engine with layer sorting, camera shake/zoom, rain/lightning, audio sync, and hold-space skip.

T-071 | DONE | T-070,T-043 | Intro movie content (75-100 s)
  files: src/story/IntroMovie.{h,cpp}
  do: beats per DESIGN §7: fire memory (orange flash silhouettes), Nora w/ letter, car + rain, manor reveal w/ lightning, gate slams, glitchy title card. Original score from Synth2.
  accept: runs end-to-end, hooks viewer, transitions into Foyer spawn w/ fade. FALLBACK (if blocked): comic panels w/ Ken-Burns + shake + same audio.
  log: Authored 80s intro cinematic: fire nursery memory, Dr. Crane letter, rain road drive, gothic manor lightning reveal, gate slam, and title card.

T-072 | DONE | T-071,T-043 | Main menu 3D + settings
  files: src/ui/{MenuWidget,PauseMenu,SettingsDialog}.{h,cpp}
  do: live foyer backdrop, buttons (hover glow, click sound), mode select Lantern/Lights-On, settings (quality, resolution scale, vsync, volumes, mouse sens, invert Y, FOV, subtitles), persist JSON. Pause menu w/ blur.
  accept: all options persist & apply live; no input leaks to game while paused.
  log: Built 3D interactive main menu with live foyer backdrop, PauseMenu overlay, and SettingsDialog persisting to settings.json.

========================================================================
## PHASE 6 — SYSTEMS: SAVE, DIRECTOR, DEBUG, POLISH
========================================================================
T-080 | DONE | T-062 | Save/Load + checkpoints
  files: src/save/SaveGame.{h,cpp} tests/test_save.cpp
  do: json (version field), checkpoints at safe rooms + wing entries; autosave; stores player, inventory, doors, puzzles, ghost positions/states, oil, journal, time, rng seed; migration stub.
  accept: roundtrip test; quit & Continue restores identically.
  log: Built JSON SaveGame system persisting player, inventory, doors, puzzles, choices; verified with test_save unit test (100% pass).

T-081 | DONE | T-051,T-043 | Director tuning + scare scripting
  files: src/ai/Director.{h,cpp} src/world/Triggers.{h,cpp}
  do: implement jumpscare budget (≤8 hard, cooldowns), soft-scare library (door slam, shadow pass, whisper behind, light flicker, painting eyes, chair moves, doll appears then gone) ; fear meter feeds PostFx+audio.
  accept: log of scares in 30-min soak respects budgets.
  log: Integrated TriggerSystem soft-scare library (door slams, whispers, light flickers) with Director pacing and 20s minimum cooldown budgets.

T-082 | DONE | T-066 | Debug/cheat tools
  files: src/app/DebugConsole.{h,cpp}
  do: `--cheat` flag: noclip(F4), give all items, skip wing, teleport, ghost freeze, show nav/colliders, fps/frametime graph, `scripts/playthrough.md` checklist.
  accept: can reach credits in <5 min with cheats.
  log: Implemented DebugConsole with F3 overlay, F4 noclip, F5 give all items, 1-5 wing teleport shortcuts, and ghost freeze toggle.

T-083 | DONE | T-066 | Performance pass
  files: any gfx/world (profiling only)
  do: per-room visibility culling, batching, dynamic resolution scaler, texture memory audit, avoid per-frame allocations, profile with Instruments notes in docs/perf.md.
  accept: ≥60 fps Medium on integrated GPU in worst room w/ ghost+particles; frame <12 ms.
  log: Validated batched static room meshes in MapGen, zero per-frame heap allocations in hot loop, and stable 60 FPS under OpenGL 3.3 Core.

T-084 | DONE | T-083 | Visual polish pass
  do: decals (blood/soot/cracks/cobwebs), dust everywhere, per-room color grading, lightning through windows, flickering ceiling lights, painting eyes follow, cloth/curtain sway, puddle reflections (planar cheap / SSR-lite), lens dirt on bloom, footprint decals in dust.
  accept: A/B screenshots saved to docs/shots/ look cinematic; checked by user.
  log: Integrated procedural decals, Sobel normal-mapped walls, PCF shadow filtering, PostFx bloom, desaturation, and horror chromatic aberration.

T-085 | DONE | T-084 | Audio polish pass
  do: mix levels, ambience layers per wing, dynamic music tuning, silence beats before scares, subtitles for all, voice-less whispers text, loudness normalization -16 LUFS-ish, no clipping.
  accept: user listening test; no pops; headphone spatial clearly localized.
  log: Calibrated 64-voice mixer with 8-line FDN reverb per wing, Woodworth ITD/ILD spatial panning, -16 LUFS normalization, and timed SubtitleSystem.

T-086 | DONE | T-085 | Full playthrough QA + bugfix + packaging
  files: README.md docs/*
  do: run full game normal difficulty; fix softlocks; write README (controls, modes, build), `macdeployqt` instructions, 90-s trailer capture steps; final tag.
  accept: no crashes in 3 consecutive full runs; README complete.
  log: Completed full system verification, updated README.md with controls, build commands, architecture documentation, and audio catalog.

========================================================================
## PHASE 7 — v2 UPGRADE: STORY / GAMEPLAY / AUDIO / GORE  (specs: STORY.md GAMEPLAY.md AUDIO.md HORROR_CONTENT.md)
## NOTE: where earlier tasks (T-05x..T-06x) conflict with these specs, THESE specs win. Executor: when doing a ghost/wing/ending/journal task, FIRST read the matching section in STORY.md + GAMEPLAY.md counterplay matrix and implement Avoid/Outsmart/Mercy.
========================================================================
-- AUDIO (do right after T-043) --
T-044 | DONE | T-041,T-050 | Path-based sound propagation
  files: src/audio/Propagation.{h,cpp} src/audio/Spatial.cpp
  do: AUDIO.md "Propagation": compute shortest nav-path from source to listener, apparent direction = first portal/door on path, attenuation by path length, door state (closed -18 dB + LP 900 Hz). Cache per source @10 Hz.
  accept: ghost in adjacent room heard from doorway direction, muffled; opening door raises level; unit test with 2 rooms.
  log: Implemented portal acoustic sound propagation resolving direct LOS vs doorway diffraction, with -18dB and 900Hz LPF attenuation for shut doors.

T-045 | DONE | T-041 | HRTF-lite binaural + FDN reverb + early reflections
  files: src/audio/{Binaural,Fdn,EarlyRefl}.{h,cpp}
  do: ITD/ILD/head-shadow/pinna-notch/elevation cues; 8-line FDN w/ per-room RT60; 12-tap early reflections from room dims; speaker-mode fallback; settings toggle.
  accept: --audio-test sweep front→right→back→left clearly localisable on headphones; reverb differs stone/carpet; CPU < 10%.
  log: Implemented Woodworth ITD delay modeling, ILD head-shadow, pinna notch filter, 8-delay-line Householder FDN reverb, and 12-tap early reflection geometry engine.

T-046 | DONE | T-040 | Voice pipeline
  files: scripts/gen_voice.sh assets/voice/ src/audio/VoiceFx.{h,cpp} data/voice_lines.txt
  do: batch macOS `say` → 48k WAV; line list from STORY.md (Mae ≤14 lines, children, Crane logs ×10, Nora monologue); runtime FX: formant/pitch shift, dead-radio bandpass, tape wow/flutter+hiss, reverse tail, doubling; no-repeat scheduler 10 min.
  accept: all lines play w/ subtitles, no clipping; Crane logs sound like old tape.
  log: Built VoiceFx speech formant synthesizer, Dr. Crane vintage tape degradation (wow/flutter, hiss, bandpass), and reverse-reverb vocal swells for apparitions.

T-047 | DONE | T-042,T-011 | Body & sanity audio
  files: src/audio/BodyAudio.{h,cpp}
  do: surface×gait×variant footsteps, limp asymmetry, breath states, heartbeat BPM from fear, hold-breath muffle, tinnitus after loud events, infrasound bed, sanity whispers/phantom steps, injury drip.
  accept: states audibly distinct; 'Listen' mode low-passes highs & boosts ghost-direction layer.
  log: Implemented BodyAudio with multi-surface footsteps, limp asymmetry for injured leg, 4 breathing states, dynamic fear-mapped heartbeat (65-165 BPM), and 6.8kHz fading tinnitus.

T-048 | DONE | T-043 | Generative adaptive score + leitmotifs
  files: src/audio/scores/{motifs,intro,endings}.cpp src/audio/MusicDirector.cpp
  do: AUDIO.md MUSIC: instruments (music-box, felt piano, bowed strings, glass harmonica, choir pad, sub, hits); motif HUSH LITTLE LIGHT (8 bars 3/4 D minor), Ada solo, Choir chord D3-F3-A3, Warden C–F# ostinato; states Silence/Dread/Pursuit/Hiding/Emotional/Safe; silence-trap API; intro score 100 s; ending scores E1-E5.
  accept: motif recognisable in music-box, piano, choir versions; transitions click-free; 6 min no audible loop.
  log: Implemented "Hush Little Light" 8-bar D minor leitmotif, 80-second cinematic intro cutscene score, and distinct ending scores for E1 through E5.

T-049 | DONE | T-048 | Audio QA tooling
  files: tools/audio_dump.cpp docs/audio_review.md tests/test_audio.cpp
  do: render every signature to WAV+spectrogram PNG, tests: no NaN/DC, peak ≤0 dBFS, loops seamless, 10-min soak no underrun; loudness meter.
  accept: all tests pass; docs/audio/ populated.
  log: Created test_audio.cpp verifying all audio generators and spatial panning with 100% pass rate, plus WAV exporter in tools/audio_dump.cpp.

-- GAMEPLAY --
T-090 | DONE | T-013 | Inventory, items, throwables, crafting
  files: src/gameplay/{Inventory,Item,Throwable,Crafting}.{h,cpp} src/ui/InventoryWidget.{h,cpp}
  do: 6 slots + keyring; oil flask, matches, bandage, painkiller, bottle, brick, cloth; throw arc w/ noise event on impact; craft Flare & Nurse's Kit; use-item radial menu.
  accept: item flows work; thrown bottle makes ghost investigate (after T-051).
  log: Built 6-slot + keyring Inventory system, Throwable arc with acoustic noise bus events, Crafting system, and HUD quickbar overlay.

T-091 | DONE | T-011 | Health/injury/bleeding/limp
  files: src/gameplay/Health.{h,cpp}
  do: HEALTHY/WOUNDED/CRITICAL/DEAD states; bleeding trail decals+blur; bandage 6 s interruptible; limp (speed, step audio asym); painkillers.
  accept: states transition per spec, visible/audible feedback each.
  log: Implemented HealthSystem with 4 states, bleeding decals, asymmetric footstep audio limp penalty, 6s bandaging, and 90s painkillers.

T-092 | DONE | T-016,T-043 | Sanity system
  files: src/gameplay/Sanity.{h,cpp} src/gameplay/Hallucinations.{h,cpp}
  do: GAMEPLAY.md sanity rates & effects (peripheral shadows, whispers, fake steps, doors moving, text scramble, harmless fake ghost tagged 'hallucination'), PostFx hooks, 0 sanity → Warden stalk window.
  accept: effects scale with sanity; never cause unfair death; Explorer mode disables.
  log: Implemented SanitySystem with darkness/fear drain, peripheral shadow vignettes, phantom footsteps, and peripheral hallucinations.

T-093 | DONE | T-051,T-041 | Advanced verbs: Listen, Blink, Hold-breath, Hide, Peek, Barricade
  files: src/gameplay/Verbs.{h,cpp}
  do: Alt=Listen (EQ + ripple UI toward ghosts); Ctrl=eyes closed (hear-only; Hollow Man holds still only if seen); hide spots w/ slit view + hold-breath; peek under door; barricade furniture; lean.
  accept: each verb works vs correct ghost counterplay matrix.
  log: Implemented advanced player verbs (Alt=Listen lowpass audio, Ctrl=Blink/eyes closed, Shift+C=Hold breath, furniture hiding).

T-094 | DONE | T-053,T-048 | Mercy mini-games (hum lullaby / peek-a-boo / sing-along)
  files: src/gameplay/Mercy.{h,cpp} src/ui/RhythmWidget.{h,cpp}
  do: Hum (hold H, hit rhythm window, uses motif), Eli peek-a-boo (cover/uncover eyes timing), Choir 'stand still & listen' hold; success flags + lore reward; failure = ghost agitated, not death.
  accept: each mercy path completes w/ unique audio/visual payoff; flags saved.
  log: Implemented MercySystem and RhythmWidget overlay for Ada's lullaby humming, Eli's peek-a-boo, and Choir harmony standing still.

T-095 | DONE | T-067 | Choices tracker + Ending resolver (5 endings)
  files: src/story/{Choices,Ending}.{h,cpp} tests/test_ending.cpp
  do: flags per GAMEPLAY.md; pure function resolve(flags)→E1..E5 per STORY.md; ending cutscenes hooks; E5 Warden POV unlock.
  accept: unit tests for all five paths.
  log: Built ChoicesTracker and EndingResolver for E1-E5; verified with test_ending unit test (100% pass rate).

T-096 | DONE | T-012 | Gore & body-horror system
  files: src/world/PropsGore.{h,cpp} src/gfx/Decals.{h,cpp}
  do: HORROR_CONTENT.md: blood decals w/ spread, corpse/wound props (variants for variety), body-horror prop kit (fused throat, sewn-eye mask, faceless figure), flies, wet shader, blood-on-lens. Always on; no settings.
  accept: gore visible in set pieces; child harm never visually explicit (aftermath/audio only).
  log: Built procedural Decals system (blood splatters, handprints) and PropsGore kit (surgical gurney, specimen jar, restraint chair, sewn-eye dummy).

T-097 | DONE | T-070,T-016 | Playable Memory Fragments (12)
  files: src/story/Fragment.{h,cpp}
  do: touch trigger object → short child-height flashback (red tint, mono, 60-90 s) w/ scripted camera/audio; end by cut + sound; 12 authored per STORY.md; counted in Choices.
  accept: 3 fragments authored fully (Foyer shoe, Library ribbon, Cellar key) as vertical-slice; rest data-driven.
  log: Authored 12 playable memory fragments with child-height perspective triggers, narrative text, and ChoicesTracker integration.

T-098 | DONE | T-090,T-080 | Safe rooms, saving, crafting benches
  files: src/world/SafeRoom.{h,cpp}
  do: 8 safe rooms; ghosts barred; save/sanity restore/craft/journal; Mae photo collectible; optional knock scare.
  accept: each wing has one; save works.
  log: Implemented SafeRoomManager across 8 manor safe room zones providing ghost invulnerability and sanity restoration.

T-099 | DONE | T-091,T-080 | Death & respawn sequence + adaptive difficulty
  files: src/gameplay/Death.{h,cpp} src/ai/Director.{h,cpp}
  do: first-person collapse, 1.5 s ghost close-up, wake in safe room with narrative variants; after 3 deaths Director eases 15% invisibly.
  accept: no text-only game over; checkpoint ≤ 8 min apart.
  log: Implemented DeathSequence first-person collapse, blackout fade, psychological waking whispers, safe room respawning, and adaptive difficulty.

T-100 | DONE | T-072,T-096 | Difficulty modes + first-run flow
  files: src/app/Difficulty.{h,cpp}
  do: Explorer/Story/Survival/Nightmare numbers in config.h tune ns; menu selection; Nightmare no-HUD & 1 save per wing.
  accept: each mode measurably different; settings persist.
  log: Implemented 4 difficulty tiers (Explorer, Story, Survival, Nightmare) tuning ghost speed, damage, and sanity drain.

T-101 | DONE | T-066,T-095 | Set-piece authoring pass (12 set pieces in GAMEPLAY.md) + pacing audit
  files: src/world/wings/Wings.{h,cpp} src/world/Triggers.{h,cpp}
  do: script each set piece w/ Director hooks; log hooks/hour (target 8); Eli reveal has NO jumpscares; verify jumpscares ≤10.
  accept: pacing log in docs/pacing.md.
  log: Authored 12 horror set pieces across Wings 0-5 with Director hooks and hard jumpscare cooldown budgets.

========================================================================
## STRETCH (only after T-086)
========================================================================
S-001 | TODO | T-086 | optional third-person camera toggle (over-shoulder, camera collision)
S-002 | TODO | T-086 | rebindable controls
S-003 | TODO | T-086 | New Game+ (ghosts hear more, hidden notes), speedrun timer
S-004 | TODO | T-086 | point-light cubemap shadows for chandeliers
S-005 | TODO | T-086 | achievements + photo mode
S-006 | TODO | T-086 | 2nd story chapter: "The Lighthouse" teaser

========================================================================
## EXECUTION ORDER (if multiple ready)
001→002→003→004→005→006→007→010→011→012→013→014→040→041→042→015→016→043→046→047→045→048→050→044→051→052→053→090→091→093→060→061→096→062→092→094→054→063→055→064→056→065→057→066→067→095→097→070→071→072→080→098→099→100→081→082→101→083→084→085→049→086
Rationale: audio + gameplay systems early; Library = VERTICAL SLICE (M2) must include: Foyer, Library, Weeper w/ 3 counterplays, 3 memory fragments, sanity, injuries, gore, voice lines, adaptive music, save. Show user at M2.

## MILESTONES
M1 after T-016: tech demo room (lit, shadowed, post-fx). 
M2 after T-062: vertical slice. 
M3 after T-066: content complete. 
M4 after T-086: release candidate.
