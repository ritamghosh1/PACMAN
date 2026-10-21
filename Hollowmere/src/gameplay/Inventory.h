#pragma once

#include "Item.h"
#include "Crafting.h"
#include <vector>

namespace hm {

class Player;

class Inventory {
public:
    static constexpr size_t MAX_SLOTS = 6;

    Inventory();

    bool addItem(const InventoryItem& item);
    bool removeItem(ItemType type, int count = 1);
    bool hasItem(ItemType type, int count = 1) const;
    bool hasKey(ItemType keyType) const;

    bool craft(const CraftingRecipe& recipe);

    const std::vector<InventoryItem>& itemSlots() const { return m_slots; }
    const std::vector<InventoryItem>& keys() const { return m_keys; }
    size_t activeSlot() const { return m_activeSlot; }
    void setActiveSlot(size_t index) { if (index < MAX_SLOTS) m_activeSlot = index; }

    int sigilsCollected() const { return m_sigilsCollected; }
    void addSigil(int id) { m_sigilsCollected++; (void)id; }

private:
    std::vector<InventoryItem> m_slots;
    std::vector<InventoryItem> m_keys;
    size_t m_activeSlot = 0;
    int m_sigilsCollected = 0;
};

} // namespace hm
