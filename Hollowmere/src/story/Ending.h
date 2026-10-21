#pragma once

#include "Cutscene.h"
#include "Choices.h"
#include "audio/AudioEngine.h"

namespace hm {

class EndingCutscene : public Cutscene {
public:
    EndingCutscene(EndingType ending, AudioEngine* audio = nullptr);

    EndingType endingType() const { return m_ending; }
    void init();

private:
    EndingType m_ending;
    AudioEngine* m_audio = nullptr;
    void setupEndingScenes();
};

} // namespace hm
