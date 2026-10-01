QT += core gui widgets multimedia

CONFIG += c++17 console sdk_no_version_check
CONFIG -= app_bundle

TEMPLATE = app
TARGET = pacman

# AGL was removed from modern macOS SDKs
QMAKE_LIBS_OPENGL = -framework OpenGL

SOURCES += \
    src/main.cpp \
    src/maze.cpp \
    src/game.cpp \
    src/sound.cpp

HEADERS += \
    src/config.h \
    src/entity.h \
    src/ghost.h \
    src/maze.h \
    src/game.h \
    src/pixel_graphics.h \
    src/sound.h
