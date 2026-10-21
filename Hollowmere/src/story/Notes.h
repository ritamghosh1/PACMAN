#pragma once

#include <string>
#include <vector>
#include "core/mathx.h"

namespace hm {

enum class WingLore {
    Foyer,
    Library,
    Conservatory,
    Cellar,
    Nursery,
    Chapel
};

struct NoteData {
    int id = 0;
    WingLore wing = WingLore::Foyer;
    std::string title;
    std::string text;
    Vec3 worldPos;
    bool collected = false;
};

class NoteLibrary {
public:
    static std::vector<NoteData> createAllNotes();
};

} // namespace hm
