#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace hm {

enum class VoiceCharacter {
    Mae,         // Young girl: high-pitched formant, ethereal, slight echo
    DrCrane,     // Asylum warden: cold, slow, heavy tape hiss, bandpass filter
    Nora,        // Protagonist: close-mic, intimate, nervous tremor
    WhisperingChild // Hallucination whisper
};

class VoiceFx {
public:
    static constexpr int SAMPLE_RATE = 44100;

    // Synthesizes an evocative vocalized dialogue phrase with character-specific acoustic processing
    static std::vector<int16_t> synthesizePhrase(const std::string& lineText, VoiceCharacter character);

    // Apply tape wow & flutter + vintage bandpass filter (for Crane audio logs)
    static std::vector<int16_t> processCraneTape(const std::vector<int16_t>& dryVoice);

    // Apply reverse-reverb vocal swell (for spooky whispers and apparitions)
    static std::vector<int16_t> applyReverseSwell(const std::vector<int16_t>& dryVoice);
};

} // namespace hm
