QT += core gui widgets multimedia opengl

CONFIG += c++17 console sdk_no_version_check
CONFIG -= app_bundle

TEMPLATE = app
TARGET = hollowmere

macx: QMAKE_LIBS_OPENGL = -framework OpenGL
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0

INCLUDEPATH += src
DEPENDPATH += src

HEADERS += \
    src/core/config.h \
    src/core/mathx.h \
    src/core/rng.h \
    src/core/Clock.h \
    src/core/Input.h \
    src/core/Log.h \
    src/gfx/GLView.h \
    src/gfx/Shader.h \
    src/gfx/Mesh.h \
    src/gfx/MeshBuilder.h \
    src/gfx/Camera.h \
    src/gfx/Light.h \
    src/gfx/Texture.h \
    src/gfx/ProcTex.h \
    src/gfx/Renderer.h \
    src/gfx/PostFx.h \
    src/gfx/Overlay2D.h \
    src/gfx/ShadowMap.h \
    src/world/Collision.h \
    src/world/Door.h \
    src/world/Pickup.h \
    src/world/MapGen.h \
    src/world/World.h \
    src/entities/Lantern.h \
    src/entities/Player.h \
    src/world/NavGrid.h \
    src/ai/Pathfind.h \
    src/ai/Senses.h \
    src/ai/Director.h \
    src/gfx/GhostRenderer.h \
    src/entities/Weeper.h \
    src/entities/HollowMan.h \
    src/entities/Choir.h \
    src/entities/Doll.h \
    src/entities/Warden.h \
    src/audio/Synth.h \
    src/audio/Synth2.h \
    src/audio/Spatial.h \
    src/audio/Fdn.h \
    src/audio/EarlyRefl.h \
    src/audio/Reverb.h \
    src/audio/Propagation.h \
    src/audio/MusicDirector.h \
    src/audio/VoiceFx.h \
    src/audio/BodyAudio.h \
    src/audio/scores/motifs.h \
    src/audio/AudioEngine.h \
    src/entities/Ghost.h \
    src/gameplay/Item.h \
    src/gameplay/Crafting.h \
    src/gameplay/Health.h \
    src/gameplay/Inventory.h \
    src/gameplay/Mercy.h \
    src/gameplay/Sanity.h \
    src/gameplay/Throwable.h \
    src/gameplay/Verbs.h \
    src/gfx/Decals.h \
    src/puzzle/Puzzle.h \
    src/puzzle/SigilAltar.h \
    src/puzzle/CipherPuzzle.h \
    src/puzzle/MirrorPuzzle.h \
    src/puzzle/ValvePuzzle.h \
    src/puzzle/MusicBoxPuzzle.h \
    src/save/SaveGame.h \
    src/story/Choices.h \
    src/story/Fragment.h \
    src/story/Journal.h \
    src/story/Notes.h \
    src/story/Subtitles.h \
    src/story/Cutscene.h \
    src/story/IntroMovie.h \
    src/story/Ending.h \
    src/world/Props.h \
    src/world/PropsGore.h \
    src/world/SafeRoom.h \
    src/world/Triggers.h \
    src/world/wings/Wings.h \
    src/gameplay/Death.h \
    src/gameplay/Hallucinations.h \
    src/app/Difficulty.h \
    src/app/DebugConsole.h \
    src/ui/Hud.h \
    src/ui/SettingsDialog.h \
    src/ui/MenuWidget.h \
    src/ui/PauseMenu.h \
    src/ui/JournalWidget.h \
    src/ui/InventoryWidget.h \
    src/ui/RhythmWidget.h \
    src/app/GameState.h \
    src/app/MainWindow.h

SOURCES += \
    src/main.cpp \
    src/core/Clock.cpp \
    src/core/Input.cpp \
    src/gfx/GLView.cpp \
    src/gfx/Shader.cpp \
    src/gfx/Mesh.cpp \
    src/gfx/MeshBuilder.cpp \
    src/gfx/Camera.cpp \
    src/gfx/Light.cpp \
    src/gfx/Texture.cpp \
    src/gfx/ProcTex.cpp \
    src/gfx/Renderer.cpp \
    src/gfx/PostFx.cpp \
    src/gfx/Overlay2D.cpp \
    src/gfx/ShadowMap.cpp \
    src/gfx/GhostRenderer.cpp \
    src/gfx/Decals.cpp \
    src/world/Collision.cpp \
    src/world/Door.cpp \
    src/world/Pickup.cpp \
    src/world/MapGen.cpp \
    src/world/World.cpp \
    src/world/NavGrid.cpp \
    src/world/Props.cpp \
    src/world/PropsGore.cpp \
    src/world/SafeRoom.cpp \
    src/world/Triggers.cpp \
    src/world/wings/Wings.cpp \
    src/ai/Pathfind.cpp \
    src/ai/Senses.cpp \
    src/ai/Director.cpp \
    src/entities/Lantern.cpp \
    src/entities/Player.cpp \
    src/entities/Ghost.cpp \
    src/entities/Weeper.cpp \
    src/entities/HollowMan.cpp \
    src/entities/Choir.cpp \
    src/entities/Doll.cpp \
    src/entities/Warden.cpp \
    src/gameplay/Throwable.cpp \
    src/gameplay/Crafting.cpp \
    src/gameplay/Inventory.cpp \
    src/gameplay/Health.cpp \
    src/gameplay/Sanity.cpp \
    src/gameplay/Hallucinations.cpp \
    src/gameplay/Verbs.cpp \
    src/gameplay/Mercy.cpp \
    src/gameplay/Death.cpp \
    src/puzzle/SigilAltar.cpp \
    src/puzzle/CipherPuzzle.cpp \
    src/puzzle/MirrorPuzzle.cpp \
    src/puzzle/ValvePuzzle.cpp \
    src/puzzle/MusicBoxPuzzle.cpp \
    src/save/SaveGame.cpp \
    src/story/Choices.cpp \
    src/story/Notes.cpp \
    src/story/Fragment.cpp \
    src/story/Journal.cpp \
    src/story/Subtitles.cpp \
    src/story/Cutscene.cpp \
    src/story/IntroMovie.cpp \
    src/story/Ending.cpp \
    src/audio/Synth.cpp \
    src/audio/Synth2.cpp \
    src/audio/Spatial.cpp \
    src/audio/EarlyRefl.cpp \
    src/audio/Reverb.cpp \
    src/audio/Propagation.cpp \
    src/audio/MusicDirector.cpp \
    src/audio/VoiceFx.cpp \
    src/audio/BodyAudio.cpp \
    src/audio/scores/motifs.cpp \
    src/audio/scores/intro.cpp \
    src/audio/scores/endings.cpp \
    src/audio/AudioEngine.cpp \
    src/app/Difficulty.cpp \
    src/app/DebugConsole.cpp \
    src/ui/Hud.cpp \
    src/ui/SettingsDialog.cpp \
    src/ui/MenuWidget.cpp \
    src/ui/PauseMenu.cpp \
    src/ui/JournalWidget.cpp \
    src/ui/InventoryWidget.cpp \
    src/ui/RhythmWidget.cpp \
    src/app/MainWindow.cpp

RESOURCES += \
    resources.qrc

