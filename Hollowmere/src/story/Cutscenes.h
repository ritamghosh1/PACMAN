#pragma once

#include <memory>
#include "Cinematic.h"
#include "Choices.h"

namespace hm {

class AudioEngine;

// New-game movie: the 1998 fire, Mae's disappearance, the music box, the drive and the gate.
std::unique_ptr<Cinematic> makeIntroCinematic(AudioEngine* audio);

// One animated epilogue per ending, followed by the credits roll.
std::unique_ptr<Cinematic> makeEndingCinematic(EndingType ending, AudioEngine* audio, bool creditsOnly = false);

} // namespace hm
