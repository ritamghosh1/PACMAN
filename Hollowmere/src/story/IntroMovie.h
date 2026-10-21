#pragma once

#include "Cutscene.h"
#include "audio/AudioEngine.h"

namespace hm {

class IntroMovie : public Cutscene {
public:
    IntroMovie(AudioEngine* audio = nullptr);

    void init();

private:
    AudioEngine* m_audio = nullptr;
    void setupScenes();
};

} // namespace hm
