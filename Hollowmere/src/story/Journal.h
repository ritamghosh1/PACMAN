#pragma once

#include "Notes.h"
#include <vector>
#include <string>

namespace hm {

class Journal {
public:
    Journal();

    void addNote(const NoteData& note);
    bool hasNote(int id) const;

    std::vector<NoteData> getNotesForWing(WingLore wing) const;
    const std::vector<NoteData>& allCollectedNotes() const { return m_collectedNotes; }

    std::string getCurrentObjectiveHint(int currentWing) const;

    int totalNotesCollected() const { return static_cast<int>(m_collectedNotes.size()); }

private:
    std::vector<NoteData> m_collectedNotes;
};

} // namespace hm
