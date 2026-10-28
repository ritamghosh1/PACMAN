#pragma once

#include <memory>
#include "Cinematic.h"
#include "Choices.h"

namespace hm {

class AudioEngine;

// New-game movie: the 1998 fire, Mae's disappearance, the music box, the drive and the gate.
std::unique_ptr<Cinematic> makeIntroCinematic(AudioEngine* audio);

// A short animated flashback shown when Nora recovers memory sigil 1..4. It explains who the
// ghost of that wing was and what happened to them.
std::unique_ptr<Cinematic> makeMemoryCinematic(int sigil, AudioEngine* audio);

// One animated epilogue per ending, followed by the credits roll.
std::unique_ptr<Cinematic> makeEndingCinematic(EndingType ending, AudioEngine* audio, bool creditsOnly = false);

} // namespace hm
