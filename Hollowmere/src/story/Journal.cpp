#include "Journal.h"

namespace hm {

Journal::Journal() = default;

void Journal::addNote(const NoteData& note) {
    if (!hasNote(note.id)) {
        m_collectedNotes.push_back(note);
    }
}

bool Journal::hasNote(int id) const {
    for (const auto& n : m_collectedNotes) {
        if (n.id == id) return true;
    }
    return false;
}

std::vector<NoteData> Journal::getNotesForWing(WingLore wing) const {
    std::vector<NoteData> filtered;
    for (const auto& n : m_collectedNotes) {
        if (n.wing == wing) {
            filtered.push_back(n);
        }
    }
    return filtered;
}

std::string Journal::getCurrentObjectiveHint(int currentWing) const {
    switch (currentWing) {
        case 0:
            return "FOYER: Locate the Brass Key beneath the grandfather clock to unseal the Library door.";
        case 1:
            return "LIBRARY: Arrange the four book ciphers by date to open the hidden archive and claim the Sigil of Bone.";
        case 2:
            return "CONSERVATORY: Rotate the brass mirrors to guide the lantern beam into the crystal iris while avoiding Mr. Pell.";
        case 3:
            return "CELLAR: Balance the pressure valves to 140 PSI and vent steam to pass the blind Choir unnoticed.";
        case 4:
            return "NURSERY: Reconstruct the music box melody from the nursery chimes to calm Eli and earn the Sigil of Glass.";
        case 5:
            return "CHAPEL: Place all five Sigils upon the altar to confront the Warden and choose Nora's fate.";
        default:
            return "Survive the corridors and keep your lantern burning.";
    }
}

} // namespace hm
