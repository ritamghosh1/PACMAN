#include "Choices.h"

namespace hm {

EndingType EndingResolver::resolve(const GameChoices& choices) {
    // E5: Secret Revelation — 12 fragments + early confession
    if (choices.fragmentsFound >= 12 && choices.confessedEarly) {
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
            return "The iron gates swing open into golden morning mist. Nora gently sets down her lantern on the wet cobblestones. In her palm rests Mae's blue ribbon, warm at last. The screaming halls behind her have fallen into sacred quiet.";
        case EndingType::E2_Consumed:
            return "The ashes of Dr. Crane's records drift like black snow through the foyer. Nora feels the chill seep through her ribs until her breath no longer steams. She takes the iron keys from the floor. Someone must mind the wards.";
        case EndingType::E3_Loop:
            return "Nora bolts through the rain, slamming the car door behind her. As the ignition turns, the radio crackles to life—humming a music box in 3/4 time. Through the foggy windshield, the silhouette of the manor waits.";
        case EndingType::E4_Martyr:
            return "Standing before the altar, Nora gathers the children's tears into herself. Their small silhouettes dissolve into points of warm candlelight. They are free. The house cradles her as its silent guardian.";
        case EndingType::E5_Revelation:
            return "The secret drawer clicks open. Inside, a yellowed medical chart in Nora's own handwriting: 'Patient 404 — Nora Crane'. The truth washes over the shadows like daylight over broken glass.";
    }
    return "";
}

} // namespace hm
