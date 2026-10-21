#pragma once

#include <string>

namespace hm {

enum class ItemType {
    None,
    OilFlask,
    Matches,
    Bandage,
    Painkiller,
    Bottle,
    Brick,
    Cloth,
    Flare,
    NursesKit,
    BrassKey,
    LibraryKey,
    CellarKey,
    NurseryKey,
    ChapelSigil
};

struct InventoryItem {
    ItemType type = ItemType::None;
    std::string name;
    std::string description;
    int count = 1;
    bool isKey = false;
    int sigilId = -1; // 0 to 4 for chapel sigils
};

} // namespace hm
