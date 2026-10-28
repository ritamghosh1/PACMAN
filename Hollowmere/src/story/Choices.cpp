#include "Choices.h"

namespace hm {

EndingType EndingResolver::resolve(const GameChoices& choices) {
    // E5: Secret Revelation — 12 fragments + early confession
    if (choices.fragmentsFound >= 16 && choices.confessedEarly) {
        return EndingType::E5_Revelation;
    }

    // E1: Release — Spared Ada & Eli & Choir, found at least 8 fragments
    if (choices.sparedAda && choices.sparedEli && choices.sparedChoir && choices.fragmentsFound >= 8) {
        return EndingType::E1_Release;
    }

    // E4: Martyr — Spared all ghosts but missed fragments
    if (choices.sparedAda && choices.sparedEli && choices.sparedChoir) {
        return EndingType::E4_Martyr;
    }

    // E2: Consumed — Burned files, banished/killed ghosts
    if (choices.burnedFiles || choices.ghostsKilledOrBanished >= 3) {
        return EndingType::E2_Consumed;
    }

    // Default: E3 — Trapped in the Loop
    return EndingType::E3_Loop;
}

std::string EndingResolver::getEndingTitle(EndingType ending) {
    switch (ending) {
        case EndingType::E1_Release:    return "ENDING I — THE DAWN AT HOLLOWMERE";
        case EndingType::E2_Consumed:   return "ENDING II — THE NEW CARETAKER";
        case EndingType::E3_Loop:       return "ENDING III — THE ENDLESS DRIVE";
        case EndingType::E4_Martyr:     return "ENDING IV — CHORUS OF THE INNOCENT";
        case EndingType::E5_Revelation: return "ENDING V — THE DOCTOR'S ARCHIVE";
    }
    return "UNKNOWN ENDING";
}

std::string EndingResolver::getEndingEpilogue(EndingType ending) {
    switch (ending) {
        case EndingType::E1_Release:
            return "The sigils settle into the altar and the ward door swings open. Nora says it aloud at last: she locked them in, and she is sorry. The children forgive her. The house burns away into morning, and when she wakes, Mae's red ribbon is warm in her hand.";
        case EndingType::E2_Consumed:
            return "Ash drifts through the foyer like black snow. The cold seeps into Nora until her breath no longer steams. She lifts the iron keys from the floor. Someone must mind the wards.";
        case EndingType::E3_Loop:
            return "Nora drags Mae to the car and drives without looking back. The radio crackles into a music box, a lullaby in three-four time. In the mirror, the manor waits. The road remembers her.";
        case EndingType::E4_Martyr:
            return "Nora kneels at the altar and sings the children to rest. Their small shapes dissolve into candlelight, and Mae walks out alone into the dawn. The house keeps Nora as its quiet guardian.";
        case EndingType::E5_Revelation:
            return "The hidden drawer clicks open. Inside lies a yellowed medical chart in Dr. Crane's hand: Patient 404, Nora Hale. The truth washes over the shadows like daylight over broken glass.";
    }
    return "";
}

} // namespace hm
