#pragma once

#include <string>

namespace hm {

enum class EndingType {
    E1_Release,    // True Ending: Forgiveness & Dawn
    E2_Consumed,   // Bad Ending: Consumed by the Hollow
    E3_Loop,       // Neutral Ending: The Endless Corridor
    E4_Martyr,     // Bitter-sweet Ending: The Choral Sacrifice
    E5_Revelation  // Secret Ending: The Warden's Truth
};

struct GameChoices {
    bool sparedAda = false;
    bool sparedEli = false;
    bool sparedChoir = false;
    bool burnedFiles = false;
    bool confessedEarly = false;
    int fragmentsFound = 0; // 0 to 12
    int maePhotosFound = 0; // 0 to 5
    int ghostsKilledOrBanished = 0;
};

class EndingResolver {
public:
    static EndingType resolve(const GameChoices& choices);
    static std::string getEndingTitle(EndingType ending);
    static std::string getEndingEpilogue(EndingType ending);
};

} // namespace hm
