# AUDIO.md — Hollowmere sound bible (extends/overrides DESIGN §8). Sound is the #1 feature.
# All audio procedural or tool-generated at dev time; zero downloaded assets.

## PHILOSOPHY
- 70% of fear is heard. The player should be able to play with the screen black and know where danger is.
- Contrast: quiet = tension; silence before payoff; never constant loudness.
- Every ghost has an acoustic signature the player learns to read.
- Spatial accuracy over spectacle: wrong localisation breaks horror.
- Headphones-first mix, with speaker mode fallback.

## ENGINE (T-040/041/044) — hm::AudioEngine
- 48 kHz internal float, stereo out (QAudioOutput); optional "binaural" mode. 128 voices, voice stealing by priority.
- Spatialiser: HRTF-lite binaural (ITD up to 0.65 ms + ILD + head-shadow lowpass ~ azimuth + pinna notch cue for front/back via small comb filter at 6-9 kHz + elevation cue). Speaker mode = simple equal-power pan.
- Distance: inverse-distance with floor, air-absorption lowpass (cutoff = 20k / (1+d*0.35)), reverb send rises with distance.
- Propagation (key innovation): sound travels via NAV PATH, not straight line — occlusion + "apparent direction" through the nearest door/opening (portal), so a ghost in the next room is heard coming from the doorway, muffled; open doors increase level, closed doors cut ~18 dB + lowpass 900 Hz. Implement on NavGrid graph (T-044).
- Room acoustics: per-room RT60/size/absorption → feedback-delay-network (8-line FDN) reverb, early reflections via 12-tap delay from room dims; crossfade between rooms across doorways. Wet/dry ties to material (stone long, carpet dry, glass bright, water slap).
- Dynamics: master bus compressor + brickwall limiter; sidechain duck of ambience/music under dialogue and hard stingers; loudness target ≈ -18 LUFS integrated, peaks ≤ -1 dBTP.
- Event bus: audio reads gameplay state (fear, sanity, intensity, hiding, injured, lantern state, room id).
- Latency < 40 ms; no allocations in audio callback; lock-free command queue.

## PLAYER BODY AUDIO (always present, adaptive)
- Footsteps: per surface (wood, creaky wood, stone, carpet, wet stone/water, metal grate, glass debris, gravel) × crouch/walk/sprint × 6 variants, random pitch ±4%, heel-toe two-part transient, gear rustle (nurse keys).
- Breath: layered loops (calm/winded/terrified/hold-breath exhale); hold-breath = heartbeat prominence + hearing muffled.
- Heartbeat: lub-dub w/ BPM 60-170 mapped to fear; low-pass when injured; drum sync with chase music.
- Injury: wet cloth drip, limp step asymmetry (one foot heavier), pain gasp.
- Sanity: whispers (formant-synth), phantom footsteps, reversed vocal swells, intermodulation hum, tinnitus 6.8 kHz sine fade after loud events, bass "pressure" 18-22 Hz infrasound (very low level, constant under dread).

## WORLD AUDIO
- Ambience beds per wing (3-5 layers each, randomised one-shots w/ non-repeating scheduler): house creaks (thermal), distant pipes, rain on glass/slate (convolved w/ room), wind through gaps, clock tick, dripping, insects in conservatory, steam & water in cellar, fire crackle in chapel candles, radio static in safe rooms.
- Interactables: door creak library (6 variants speed-dependent: slow/fast), latch, key rattle, drawer, paper, glass, wood stress, pickup chimes (subtle, diegetic), lantern squeak/click/hiss of flame, oil slosh, match strike (noise burst + sulphur crackle), bandage rip, bottle break, brick thump, locker clang, lever clunk, valve squeal, steam burst.
- Structure sounds: house "breathing" every 40-90 s, distant slam, ceiling footsteps (ghost upstairs, positional), pipes knocking in code (hint for puzzle!).
- Weather: storm with lightning → thunder delay by distance (0.5–6 s), flash lights windows.

## GHOST SIGNATURES (learnable)
| Weeper | wet quavering vocal-synth (FM + formants) rising when close; breath-intake before wail; blood-drip; muffled wail through walls; her sobbing syncs to her state (calm = soft hiccup). |
| Hollow Man | NO footsteps when moving (!); only pruning-shears "snip" every ~3 s + tight wind ‘shh’; static burst + subsonic hit when he is seen / freezes; silence pocket around him (ambience ducks 6 dB). |
| Choir | 3-note chord on stable pitches (e.g. D3-F3-A3 minor); chord detunes/fractures when aggravated; one voice stops when it spots you; breath rasps from fused throats; wet clicks. |
| Doll/Eli | music-box motif; muffled giggles; counting "one… two…" tempo rises as he nears; feet patter when hunting; porcelain click; lullaby when calm. |
| Warden | iron keys jingle (rhythm = distance), heavy dragging chain, bass drone w/ distortion, door bashes; fire roar bed during chase; low-frequency rumble floor shake. |

## MUSIC (T-043, T-043b) — generative, adaptive, leitmotif-based
- Instrumentation synthesised: music-box (inharmonic additive bell), felt piano (hammer noise + decaying partials), bowed cello & strings (saw bank + LPF + vibrato + bow noise), glass harmonica (sine cluster), prepared-piano percussion, deep taiko-ish hits, sub-bass, choir pad (formant), granular textures from room noise.
- Leitmotifs: HUSH LITTLE LIGHT (Nora/Mae, 8 bars 3/4, minor) — appears as music box (safe/emotional), distorted (Doll), whistled (Pell), choral (finale). Ada: solo voice. Choir: chord. Warden: low tritone ostinato (C – F#).
- Stems & states: Silence | Dread bed (2 layers) | Pursuit (percussion, strings ostinato) | Hiding (muted, heartbeat-forward) | Emotional (Eli, finale: piano+strings, no scares) | Safe room (distant radio-lullaby) | Combat-lull.
- Music director rules: min 20 s between cues; stingers quantised to nearest beat within 80 ms only if not silence-trap; during silence traps (planned) music hard-stops 2-4 s → scare/hush; after chase, drone decays with reverb tail.
- Generative: Markov-ish note selection constrained to D minor/Phrygian pool, voice leading rules, never loops audibly > 6 min; seed from game state.
- Cinematic cues: intro score 100 s full composition coded as note-lists (src/audio/scores/intro.cpp), ending scores E1-E5 (E1 resolves to major 3rd on final chord).

## VOICE
- (Superseded: the game has no voice acting. See README section 4.) Old plan, kept for history: scripts/gen_voice.sh used macOS `say` (e.g. voices Samantha/Daniel/Whisper) → AIFF → resample 48k WAV into assets/voice/ (committed, small). Runtime FX: formant shift, pitch, reverb, "dead radio" bandpass, reversed tail, doubling → used for Mae, children, Crane audio logs, Nora monologue. Nora's monologue can be subtitle-only if voice quality poor.
- ≤ 80 voice lines total; Crane's audio logs (10) = cold, slow, 70 bpm cadence, close-mic dry w/ tape hiss & wow/flutter.
- Subtitles + sound captions mandatory.

## MIX PRIORITY (ducking order)
1 Voice/story  2 Ghost signature  3 Player body  4 Interactions  5 Music  6 Ambience. Hard stingers duck all by 8 dB for 400 ms.
Dynamic range: allow 20 dB difference quiet↔loud; never clip.

## QA
- `--audio-test` suite: each signature plays with label; spectrogram dump (PNG) saved to docs/audio/ for review; automatic tests: no NaN, no DC offset, peak ≤ 0 dBFS, loops seamless (zero-crossing/xfade check), 10-min soak no underrun.
- Listening checklist docs/audio_review.md: front/back, near/far, behind-wall, door open/close sweep, left/right panning, headphones vs speakers.
