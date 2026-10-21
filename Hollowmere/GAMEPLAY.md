# GAMEPLAY.md — systems that make Hollowmere play better than "walk, hide, die"
# (extends DESIGN §3 & §6; if conflict, THIS file wins)

## DESIGN GOALS
1. Every second is a decision: light vs stealth vs speed vs mercy.
2. Ghosts are puzzles with personalities, each solvable 3 ways: AVOID / OUTSMART / MERCY (emotional).
3. Fair, readable fear: telegraphs, consistent rules, no cheap deaths.
4. Resource tension: oil, bandages, matches, stamina, sanity — never all abundant.
5. Player agency: choices tracked → 5 endings.

## CORE VERBS
walk / sprint / crouch / lean(Q,E) / peek-under-door (crouch at door) / interact / pick-up / throw (distraction) / use item / lantern on-off / lantern FOCUS (RMB: narrow bright beam, 3x oil) / HOLD BREATH (Shift+C while hidden) / BLINK-CLOSE-EYES (hold Ctrl: screen black; hear-only; used vs Hollow Man) / LISTEN (hold Alt: audio-eq cut highs, enhance ghost direction w/ faint ripple UI; drains sanity slightly) / HUM (hold H near Ada/Eli: rhythm mini-game to calm) / barricade doors (pull furniture; noise) / hide (lockers, beds, wardrobes, under tables, curtains).

## RESOURCES
- Oil (0-100): drains lit; refills (flask +35). Lights-On mode: infinite.
- Matches (0-9): 8 s dim flame; light in dead lantern emergencies; wind/water extinguishes.
- Bandages (0-5): stop bleeding (blur + footprints + sound), 6 s apply (interruptible; can apply while hiding).
- Painkillers (0-3): negate limp for 90 s.
- Glass shards / bottles / bricks: throwables (noise radius 8 at impact; ghost investigates).
- Batteries: NONE. (Oil/matches only: tense).
- Inventory: 6 slots + key ring (unlimited) + notes (journal). Crafting (simple): bottle + cloth + oil = Flare (bright 8 s, repels Weeper/Choir); bandage + painkiller = "Nurse's Kit" (heal+limp).
- Nora's nurse skill: bandaging 30% faster; can read patient charts (extra hints).

## HEALTH, INJURY, SANITY
Health model = states, not bar: HEALTHY → WOUNDED (bleeding, 1 hit more fatal) → CRITICAL (limp, vision pulse) → DEAD (checkpoint). Ghost hit = wounded once unless "telegraph failed".
Sanity (0-100): drops in dark, near ghosts, seeing horrors (corpses), listening mode; regained in safe rooms, with lullaby hum, lit rooms, photographs of Mae (collectible).
Low sanity effects (progressive; ALL purely audiovisual, never random death): peripheral shadows, breathing wrong, whispers saying Nora's name, door moves when not looking, fake footsteps behind, doubled lantern shadow, wall breathing, text of notes scrambles, fake ghost sighting (harmless, "hallucination" tagged in subtitles log for fairness). At 0: Warden senses you — the Director spawns a stalker window.

## STEALTH & GHOST COUNTERPLAY MATRIX
| ghost | avoid | outsmart | mercy |
| Weeper | stay quiet, don't sprint | flare/lantern focus drives off | hum lullaby → she sits, leaves a clue |
| Hollow Man | break line-of-sight, doors | lock him in glass room; blink to hold | none (he's not a child) |
| Choir | sneak, lockers | throw noise to lead away; steam vents knock them out | stand still, let them sing → they part |
| Doll (Eli) | stay lit, avoid dark | stun w/ light 3 s | peek-a-boo mini-game, he gives items |
| Warden | run, doors | collapse hazards, blind burst | confession (ending only) |
Mercy options are slower, riskier, give lore + better ending. Player pick = playstyle expression.

## EXPLORATION & PUZZLES
- Hub-and-spoke: Foyer hub; wings 1-4 any order after intro (first = Library recommended via hints), Chapel locked behind 4 sigils (+Mae's drawing).
- Shortcuts unlock after first pass (keys, levers, shelves). Loops shorten return trips.
- Puzzles 3-4 steps, use sound/light/meaning; each has a "gentle hint" after 4 min stuck (journal nudge), never marker spam.
- Optional rooms with lore & upgrades: "Matron's Office" (lantern upgrade: wider cone), "Dispensary" (painkillers), "Crane's Lab" (gore + true-ending clue).
- Secrets: 12 memory fragments; 9 "red crayon 9" signs (9/9 → unlock hidden room with E1 clue).
- Replay hooks: seeded variants of prop placement & some ghost routes (NOT story-critical).

## MORALITY / CHOICE TRACKER (src/story/Choices.{h,cpp}, saved)
flags: spared_ada, harmed_eli, burned_files, saved_cellar_child, found_fragments(0-12), confessed_early, hum_calmed_ghosts(0-3), mae_photos(0-5).
Ending resolver in Ending.cpp = pure function of flags (unit-tested). Show NO moral meter; only subtle sound/visual feedback.

## DIFFICULTY (affects numbers, not content)
EXPLORER (lights-on, ghosts -25% speed, extra oil, no sanity loss) / STORY / SURVIVAL (default) / NIGHTMARE (no HUD, less oil, ghosts hear +20%, 1 save per wing, unlocks E5 after clear).
Adaptive: Director reduces pressure after 3 deaths in a row (invisible).

## GAME FEEL (non-negotiable polish)
- Input latency < 1 frame of buffering; mouse smoothing OFF default, raw delta.
- Camera: weight on landing, breathing sway, subtle roll on turn, FOV 80 default (slider 60-100), hand/lantern inertia & collision with walls (lantern tilts away).
- Doors: slow-open physics (hold E to creep door open silently; tap = fast & loud), lean to peek.
- Object interaction: grab & carry heavy items w/ weight slowdown; drop = noise.
- Hit reactions: camera kick, hearing muffled 2 s (low-pass + tinnitus), heartbeat swell.
- Every interaction has at least: animation, sound, particle or light response.
- Death: not a text screen — first-person fall, ghost close-up held 1.5 s, cut, wake up in safe room (with narrative text variants).

## SAFE ROOMS (8 total)
Candles + radio static humming Nora's lullaby backwards; save; craft; read journal; sanity restore; ghosts can't enter (but may knock — scare option). Each safe room has unique decor and a Mae photograph.

## SET PIECES (≥ 12, each hand-authored, 3-8 min)
1 Foyer: lights die, Mae's voice through vents.  2 Library: shelves rearrange while you read cipher.  3 Library: Weeper vs flare in aisle.  4 Conservatory: glass breaks, Hollow Man stalks through fog (eyes-closed section).  5 Pruning room: lore + gore reveal.  6 Cellar: flooded corridor with steam timing.  7 Chorus Ward: choir in dark, sing-along choice.  8 Attic: hide-and-seek with Eli (counting, sound-only tracking).  9 Eli's reveal (silent, lullaby, no scare).  10 Chapel: sigils & Mae.  11 Warden chase (collapsing house).  12 Door choice finale.

## BALANCE KNOBS (all in src/core/config.h under namespace tune)
walk 1.8, sprint 3.4, crouch 0.9; ghost speeds W 1.4/ H 3.0 / C 1.6 / D 2.6 / Wd 3.6; hearing radii; oil drain; sanity rates; telegraph 1.5 s; breather 40 s; checkpoint spacing ≤ 8 min.
Playtest log scripts/balance.md updated every milestone.
