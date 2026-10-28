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
        case 0: return "FOYER: the safe hub. Follow the gold arrow to your objective.";
        case 1: return "LIBRARY: Ada weeps here. Creep (C), or hold H beside her to hum the lullaby.";
        case 2: return "CONSERVATORY: Mr. Pell freezes while your lantern lights him. Keep him in view.";
        case 3: return "CELLAR: the Choir is blind. Walk, don't run; hold breath (SHIFT+C) when close.";
        case 4: return "NURSERY: Eli wants a game. Close your eyes (CTRL) near him, then open them.";
        case 5: return "CHAPEL: bring all four sigils to the altar. Run from the Warden.";
        default: return "Survive the corridors and keep your lantern burning.";
    }
}

} // namespace hm
