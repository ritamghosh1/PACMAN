# AGENTS.md — READ FIRST, EVERY SESSION
# audience: executor LLM (low-cost, long-context). style: terse, literal, machine-first.

## 0. IDENTITY
project: HOLLOWMERE  (single-player 3D first-person horror-maze, Qt5 + OpenGL)
root:    /Users/ritamghosh/Desktop/Class_Notes/CG/Hollowmere   <- ONLY writable dir
FORBIDDEN: any edit outside Hollowmere/ . Hollowmere is a brand-new standalone project: no code, theme, art or naming from any previous project.
FORBIDDEN: editing ../idea.txt

## 1. SESSION PROTOCOL (follow in order, no skipping)
1. read AGENTS.md (this), DESIGN.md (tech/arch), STORY.md, GAMEPLAY.md, AUDIO.md, HORROR_CONTENT.md (as relevant to task), then TODO.md (tasks).
2. pick the FIRST task in TODO.md with status=TODO whose `deps` are all DONE.
3. do ONLY that task. never batch >1 task. never "improve" unrelated code.
4. build:  cd build && cmake --build . -j  (or qmake path, see §3). MUST compile with 0 errors, 0 new warnings.
5. run acceptance checks listed in the task (`accept:`). if one fails -> fix; do not mark DONE.
6. edit TODO.md: status=DONE, add `log:` one line (what changed, files). commit: `git add Hollowmere && git commit -m "T-xxx <title>"`.
7. if blocked >3 attempts: set status=BLOCKED, write `blocker:` reason in TODO.md, move on to next unblocked task.

## 2. HARD RULES
- C++17. Qt 5.15.2 (found at /opt/anaconda3 via `qmake`). Modules: core gui widgets multimedia opengl.
- OpenGL: request 3.3 Core profile (macOS max = 4.1 core). GLSL `#version 330 core`. NO fixed-function GL. NO deprecated calls.
- Use QOpenGLWidget/QOpenGLFunctions_3_3_Core, QOpenGLShaderProgram, QOpenGLBuffer, QOpenGLVertexArrayObject, QOpenGLFramebufferObject.
- NO external assets/downloads/libs (no assimp, glm, SDL, FMOD, png/obj files). Everything PROCEDURAL:
  geometry = generated in code; textures = generated in code (noise/QImage) ; audio = synthesized into WAV buffers in code ; fonts = Qt default or QFontDatabase system fonts.
  Small own math lib: src/core/mathx.h (vec2/3/4, mat4, quat optional). Do not add glm.
- Shaders live in shaders/*.vert|*.frag, embedded via resources.qrc (":/shaders/..."). Hot-compile errors must print log via qWarning and abort loudly.
- One class per .h/.cpp pair. Header guards `#pragma once`. Namespaces: `hm::`. No raw `new` without Qt parent or smart ptr.
- NO globals except constants in src/core/config.h. NO magic numbers: put tunables in config.h or data/*.json-like constexpr tables.
- Fixed timestep sim 120 Hz (dt=1/120), render interpolated, vsync on. Never use frame dt directly for physics.
- Keep every file < 600 lines. If bigger -> split.
- Comments: why not what. Public API doc-comment 1 line.
- Do not delete or rewrite files not owned by current task. Do not rename public APIs listed in DESIGN.md §ARCH without updating all call sites in same commit.
- Determinism: all randomness via hm::Rng(seed) in src/core/rng.h. Never rand()/QRandomGenerator::global() in gameplay.
- Performance budget (MacBook integrated GPU): 60 fps @ 1280x720, <= 1500 draw calls... target < 300 via batching per room. Frame cost target < 12 ms.
- NO voice lines: characters are told apart by typewriter-tick timbre and motifs (audio/CineSfx.*). Everything stays procedural.
- Gore/disturbing content is always on (core of the game); follow HORROR_CONTENT.md authoring rules. No content toggles, gates, or rating labels.
- Save game: QSettings-free; JSON file in QStandardPaths::AppDataLocation/save.json.

## 3. BUILD
Primary: CMake is NOT installed. USE qmake (hollowmere.pro). 
```
cd /Users/ritamghosh/Desktop/Class_Notes/CG/Hollowmere
mkdir -p build && cd build && qmake ../hollowmere.pro CONFIG+=debug && make -j8
./hollowmere            # (CONFIG -= app_bundle)
```
macOS: `QMAKE_LIBS_OPENGL = -framework OpenGL` ; set `QSurfaceFormat` default BEFORE QApplication constructed: version 3.3, CoreProfile, depth 24, stencil 8, samples 4, swapInterval 1.
Headless test targets: tests/*.cpp built via `tests/tests.pro` (QtTest). Run `make check`-style script `scripts/run_tests.sh`.

## 4. DEFINITION OF DONE (global)
compiles clean; game launches; no GL errors (`glGetError` loop in debug each frame prints); no crash on window resize; ESC pauses; 60fps; feature visible/audible as per task `accept:`.

## 5. ANTI-FAILURE NOTES (common weak-model mistakes)
- do not invent Qt APIs; if unsure grep headers in /opt/anaconda3/include/qt/ .
- QOpenGLWidget: all GL calls ONLY inside initializeGL/paintGL/resizeGL or after makeCurrent().
- macOS Core profile needs a bound VAO before any draw. Always.
- Retina: use devicePixelRatioF() for glViewport & FBO sizes.
- `QAudioOutput`/`QSoundEffect` loops: for synthesized audio use QAudioOutput(QAudioFormat) pull-mode with own mixer (see T-040) — do not write temp wav files.
- mouse look: use setMouseTracking + QCursor::setPos recentre + hide cursor; on focus loss release.
- Do not block main thread (no sleeps). Loading screens = staged init across frames.
