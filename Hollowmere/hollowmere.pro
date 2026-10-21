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
    src/world/Collision.h \
    src/world/Door.h \
    src/world/Pickup.h \
    src/world/MapGen.h \
    src/world/World.h \
    src/entities/Lantern.h \
    src/entities/Player.h \
    src/entities/Ghost.h \
    src/audio/Synth.h \
    src/audio/AudioEngine.h \
    src/ui/Hud.h \
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
    src/world/Collision.cpp \
    src/world/Door.cpp \
    src/world/Pickup.cpp \
    src/world/MapGen.cpp \
    src/world/World.cpp \
    src/entities/Lantern.cpp \
    src/entities/Player.cpp \
    src/entities/Ghost.cpp \
    src/audio/Synth.cpp \
    src/audio/AudioEngine.cpp \
    src/ui/Hud.cpp \
    src/app/MainWindow.cpp

RESOURCES += \
    resources.qrc
