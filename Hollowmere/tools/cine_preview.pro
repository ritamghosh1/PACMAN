# Offline cutscene preview (no OpenGL, no audio device):  qmake tools/cine_preview.pro && make
QT += core gui multimedia
CONFIG += c++17 console
CONFIG -= app_bundle
TEMPLATE = app
TARGET = cine_preview
INCLUDEPATH += ../src
SOURCES += cine_preview.cpp \
    ../src/story/Cinematic.cpp ../src/story/Dialogue.cpp ../src/story/CineArt.cpp ../src/story/Cutscenes.cpp \
    ../src/story/Choices.cpp ../src/story/CutscenesEnding.cpp ../src/story/CutscenesMemory.cpp \
    ../src/audio/AudioEngine.cpp ../src/audio/CineSfx.cpp ../src/audio/Synth.cpp ../src/audio/Synth2.cpp \
    ../src/audio/Spatial.cpp ../src/audio/Reverb.cpp ../src/audio/EarlyRefl.cpp ../src/audio/Propagation.cpp \
    ../src/audio/MusicDirector.cpp ../src/audio/BodyAudio.cpp ../src/audio/scores/intro.cpp \
    ../src/audio/scores/endings.cpp ../src/audio/scores/motifs.cpp
HEADERS += ../src/audio/AudioEngine.h
