# qmake tests/test_intro.pro && make && ./test_intro
QT += core gui multimedia
CONFIG += c++17 console
CONFIG -= app_bundle
TARGET = test_intro
INCLUDEPATH += ../src
SOURCES += test_intro.cpp \
    ../src/story/Cinematic.cpp ../src/story/Dialogue.cpp ../src/story/CineArt.cpp ../src/story/Cutscenes.cpp ../src/story/CutscenesEnding.cpp ../src/story/CutscenesMemory.cpp ../src/story/Choices.cpp \
    ../src/audio/AudioEngine.cpp ../src/audio/CineSfx.cpp ../src/audio/Synth.cpp ../src/audio/Synth2.cpp \
    ../src/audio/Spatial.cpp ../src/audio/Reverb.cpp ../src/audio/EarlyRefl.cpp ../src/audio/Propagation.cpp \
    ../src/audio/MusicDirector.cpp ../src/audio/BodyAudio.cpp ../src/audio/scores/intro.cpp \
    ../src/audio/scores/endings.cpp ../src/audio/scores/motifs.cpp
HEADERS += ../src/audio/AudioEngine.h
