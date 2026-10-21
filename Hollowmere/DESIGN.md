# DESIGN.md — HOLLOWMERE  (spec; source of truth for all tasks)

## 1. PITCH
"Hollowmere": first-person psychological survival-horror. A mother searches the burned orphanage-asylum of her own repressed childhood for her missing daughter; the house is a living maze built from the children she left behind.
Pillars: (1) SOUND-led dread (AUDIO.md) (2) light = life, every ghost has Avoid/Outsmart/Mercy answers (GAMEPLAY.md) (3) emotional story with 5 endings (STORY.md) (4) uncompromising visceral tone (HORROR_CONTENT.md).
Length: 6-9 h story, single continuous campaign, autosave at safe rooms. No levels menu.
Reference vibes (do not copy IP): Amnesia, Alien Isolation, Silent Hill 2, Outlast, Visage, P.T., Layers of Fear.

## 2. STORY -> see STORY.md (canonical). Wing/ghost table below is a summary only.
| id | wing | ghost | mercy |
|W0| Foyer hub | scripted | - |
|W1| Library / Records | THE WEEPER (Ada) | hum lullaby |
|W2| Conservatory / Pruning garden | THE HOLLOW MAN (Pell) | none, lock him in glass |
|W3| Boiler Cellar / Chorus Ward | THE CHOIR | let them sing |
|W4| Nursery / Attic | THE DOLL (Eli) | peek-a-boo |
|W5| Chapel | THE WARDEN (Dr. Crane) | confession |

## 3. MODES
- LANTERN MODE (default, "Nightmare"): only local light. Lantern cone (spot) + small ambient radius, rest pitch black. Oil depletes (5 min full); refills found; flicker when low. Ghosts react to light.
- LIGHTS-ON MODE ("Explorer"): whole wing lit with soft ambient, no oil limit, ghosts slower 25%. Same story. Select in main menu; switchable at checkpoints.
- Camera: first-person ONLY (v1). Extra verbs/resources/sanity/injury: GAMEPLAY.md. Optional third-person = stretch task S-001.
- Movement: continuous (no grid). Walk, sprint (stamina; noise), crouch (quiet), lean(Q/E optional stretch), interact (E/click), lantern toggle (F), pause (Esc), journal (TAB).
- Walls are real meshes; map authored on a 2D tile grid of 2.0 m cells for GENERATION ONLY (grid -> meshes + nav graph). Player/ghosts use continuous physics (circle-vs-AABB sliding). NO 5px blocks.

## 4. ARCHITECTURE (names are API; do not rename)
```
Hollowmere/
  hollowmere.pro  resources.qrc  AGENTS.md DESIGN.md TODO.md
  shaders/        *.vert *.frag
  src/
    main.cpp                      # QSurfaceFormat, QApplication, MainWindow
    app/    MainWindow.{h,cpp}    # QStackedWidget: Menu | GLView | Cutscene ; owns Game
            GameState.{h,cpp}     # enum {Boot,Menu,Intro,Playing,Paused,Dead,Ending}
    core/   config.h mathx.h rng.h Clock.{h,cpp}(fixed-step) Input.{h,cpp} Log.h Json.h(thin QJson wrapper)
    gfx/    Renderer.{h,cpp}      # owns passes + FBOs
            Shader.{h,cpp}  Mesh.{h,cpp}  MeshBuilder.{h,cpp}(boxes,quads,cylinders,props)
            Texture.{h,cpp}  ProcTex.{h,cpp}(noise: wood, stone, plaster, rust, wallpaper, carpet, tiles)
            Camera.{h,cpp}  Light.{h,cpp}  ShadowMap.{h,cpp}  PostFx.{h,cpp}  Particles.{h,cpp}  TextRenderer.{h,cpp}
    world/  World.{h,cpp}  Wing.{h,cpp}  MapGen.{h,cpp}(grid->mesh)  Collision.{h,cpp}  NavGrid.{h,cpp}
            Props.{h,cpp}  Door.{h,cpp}  Interactable.{h,cpp}  Pickup.{h,cpp}  Triggers.{h,cpp}
            wings/ Foyer.cpp Library.cpp Conservatory.cpp Cellar.cpp Nursery.cpp Chapel.cpp  # layout + props + puzzle wiring each
    entities/ Player.{h,cpp}  Ghost.{h,cpp}(base FSM)  Weeper.cpp HollowMan.cpp Choir.cpp Doll.cpp Warden.cpp
    ai/     Senses.{h,cpp}(sight,hearing,light)  Pathfind.{h,cpp}(A* on NavGrid + funnel/smoothing)  Director.{h,cpp}(pacing AI)
    puzzle/ Puzzle.h(iface) CipherPuzzle ValvePuzzle MirrorPuzzle MusicBoxPuzzle SigilAltar
    audio/  AudioEngine.{h,cpp}(mixer, QAudioOutput pull) Synth.{h,cpp}(procedural sfx/music) Reverb.h Spatial.{h,cpp}(3D pan/attenuation/occlusion) MusicDirector.{h,cpp}
    story/  Journal.{h,cpp} Notes.cpp(text data) Cutscene.{h,cpp}(intro movie player) IntroMovie.cpp Subtitles.{h,cpp} Ending.cpp
    ui/     MenuWidget.{h,cpp} Hud.{h,cpp} PauseMenu SettingsDialog(quality, volume, sensitivity, mode)
    save/   SaveGame.{h,cpp}
  tests/    tests.pro test_mathx.cpp test_collision.cpp test_pathfind.cpp test_synth.cpp test_save.cpp
  scripts/  run_tests.sh
```
Core interfaces (freeze early, T-005/T-020):
```
struct Transform{vec3 pos; quat rot; vec3 scale;};
class Entity{ virtual void update(double dt); virtual void render(Renderer&); vec3 pos; };
class Ghost: Entity { enum State{Idle,Patrol,Investigate,Chase,Search,Flee,Stunned,Scripted}; virtual void think(const Senses&,double dt); }
class Puzzle { virtual bool interact(Player&); virtual bool solved() const; signal solved(); virtual json save(); virtual void load(json); }
class Interactable { virtual QString prompt() const; virtual void use(Player&); AABB bounds; }
AudioEngine::play3D(SoundId, vec3 pos, float gain, float pitch=1) ; AudioEngine::playUI(SoundId)
```

## 5. GRAPHICS SPEC (aim: beat indie-AAA look using tricks)
Pipeline (forward+, deferred NOT needed):
1. Shadow pass: lantern = spot light shadow map 1024², perspective; plus up to 4 point/room lights as cubemap shadows ONLY for nearby (stretch). Use PCF 5x5 + slope bias.
2. Main pass: Blinn-Phong→upgrade to light-model with normal mapping (procedural normal maps from height noise), specular, fresnel rim on wet surfaces, fog (exp² volumetric look, tinted per wing), flicker flame light (noise-modulated intensity+color temp 2200K).
3. Volumetric god-rays: radial-march in post using depth + lantern screen pos (cheap), plus dust particles in cone.
4. Post: bloom (13-tap dual-kawase), SSAO (16 samples hemisphere, half-res, blur), vignette, film grain, chromatic aberration (scales with fear), tone-map ACES, gamma, optional CRT/VHS "scare" distortion, motion blur light. Fear meter drives: vignette, FOV squeeze (-5°), heartbeat pulse, desaturation.
5. Textures 512²-1024² procedural, mipmaps, anisotropic x8, repeat+triplanar for walls. Decals (blood, soot, cracks) via quad overlays.
6. Props: procedurally modelled (bookshelves, tables, chairs, beds, pipes, cribs, pews, statues, paintings w/ eyes-follow trick, chandeliers, candles, curtains cloth-sway in vertex shader, doors with hinge anim).
7. Ghost visuals: translucent fresnel-rim + noise-dissolve shader, vertex wobble, trailing particles, distortion halo; Hollow Man = tall faceless silhouette (box-built humanoid, smooth normals); Doll = porcelain procedural; Warden = massive tattered shape.
8. Quality presets Low/Med/High/Ultra toggle: shadows res, SSAO, bloom, MSAA, resolution scale (0.5-1.0 dynamic res to hold 60fps).
9. Dynamic resolution + frustum culling per-room + portal/room visibility (only render current+adjacent rooms).

## 6. AI SPEC
- Senses: sight (cone 100°, range 12 m, occluded by raycast on grid walls, reduced in dark if player lantern OFF/low), hearing (noise events with radius: walk 4, sprint 10, crouch 1.5, door 6, drop 8, lantern click 2), light-sense (Weeper/Warden).
- FSM+utility; A* on 0.5 m navgrid; path smoothing; door-aware; stuck detection -> repath.
- Director (AI pacing): intensity curve; ensures a "breather" ≥ 40 s after chase; spawns hint scares; scales ghost aggression with fails (rubber-banding ±15%); never kills unfairly: ghosts telegraph (audio cue 1.5 s before attack, visual cue).
- Death: not instant game-over; screen-fall animation then respawn at last checkpoint with ghost reset; 3 "deaths" shown narrative in journal; no frustration loops.
- Hiding: lockers/wardrobes (peek through slits, hold breath QTE-lite via holding key), ghosts may check lockers w/ probability.

## 7. NARRATIVE INTRO -> script in STORY.md (INTRO MOVIE SCRIPT). Tech: 2.5D paper-cut cutscene engine in Qt (see T-070/071), fallback comic panels.

## 8. AUDIO -> see AUDIO.md (canonical). Jumpscare rule: <= 10 hard scares total, 20 s+ lull before each, none in emotional beats (Eli reveal, finale).

## 9. CONTENT METRICS
~5 wings × 12-20 rooms each; Foyer 6 rooms; total ≈ 70 rooms (mix handcrafted ASCII templates + procedural variation seeded per New Game: door positions & prop placement vary; critical path fixed).
Each wing: 1 sigil, 1 puzzle (3-4 steps), 2-3 notes, 1 oil pickup, 1-2 shortcuts unlocking on return, 1 hiding spot per 3 rooms, 1 safe room (save + radio hum).
Journal: 40 notes (Crane logs / children / Nora) + 12 playable memory fragments + 4 sigil memories + choices flags (see STORY.md, GAMEPLAY.md).
Length control: wing gating ensures backtracking via Foyer; new shortcuts reduce backtrack stress.
Map ASCII legend (MapGen input): `#` wall  `.` floor  `D` door  `L` locked door(id)  `S` spawn  `K` pickup  `P` puzzle  `G` ghost-spawn  `H` hide  `C` checkpoint  `W` window  `~` water  `^` stairs(visual only).

## 10. UI/UX
Menu: Continue / New Game / Mode (Lantern|Lights-On) / Settings / Credits / Quit; 3D live backdrop of foyer w/ camera sway + dust + candle flicker + theme music. Fonts: elegant serif for story, mono for UI hints. Diegetic HUD: oil gauge on lantern model, heartbeat vignette; minimal icons; prompts "[E] Open". Pause: blur background. Journal: parchment UI w/ page curl.
Controls rebindable (stretch). Gamepad = stretch (QGamepad not available mac -> skip).

## 11. QA / ACCEPTANCE (global)
- 60fps steady on integrated GPU @ Medium; no memory growth > 5%/hour; no black-screen on resize / fullscreen toggle (F11).
- No softlock: every puzzle has fail-safe reset; sigil items cannot be lost; keys cannot be dropped in unreachable area.
- Full run-through test script (scripts/playthrough.md) completed by autoplay debug cheat `--cheat` (noclip, give items, skip wing) in < 5 min.
- Save/Load restores wings state, puzzle state, inventory, ghost state, time.
