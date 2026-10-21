#include "Inventory.h"
#include <algorithm>

namespace hm {

Inventory::Inventory() {
    m_slots.resize(MAX_SLOTS);
}

bool Inventory::addItem(const InventoryItem& item) {
    if (item.isKey) {
        // Add to separate infinite keyring
        for (auto& k : m_keys) {
            if (k.type == item.type) {
                k.count += item.count;
                return true;
            }
        }
        m_keys.push_back(item);
        return true;
    }

    if (item.type == ItemType::ChapelSigil) {
        addSigil(item.sigilId);
        return true;
    }

    // Try stacking into existing slot
    for (auto& slot : m_slots) {
        if (slot.type == item.type) {
            slot.count += item.count;
            return true;
        }
    }

    // Place into first empty slot
    for (auto& slot : m_slots) {
        if (slot.type == ItemType::None) {
            slot = item;
            return true;
        }
    }

    return false; // Inventory full
}

bool Inventory::removeItem(ItemType type, int count) {
    for (auto& slot : m_slots) {
        if (slot.type == type) {
            if (slot.count > count) {
                slot.count -= count;
                return true;
            } else if (slot.count == count) {
                slot = InventoryItem{};
                return true;
            }
        }
    }
    return false;
}

bool Inventory::hasItem(ItemType type, int count) const {
    int total = 0;
    for (const auto& slot : m_slots) {
        if (slot.type == type) {
            total += slot.count;
            if (total >= count) return true;
        }
    }
    return false;
}

bool Inventory::hasKey(ItemType keyType) const {
    for (const auto& k : m_keys) {
        if (k.type == keyType) return true;
    }
    return false;
}

bool Inventory::craft(const CraftingRecipe& recipe) {
    if (!CraftingSystem::canCraft(m_slots, recipe)) return false;

    // Deduct ingredients
    for (ItemType req : recipe.ingredients) {
        removeItem(req, 1);
    }

    // Add result
    InventoryItem res;
    res.type = recipe.result;
    res.count = recipe.resultCount;
    if (recipe.result == ItemType::Flare) {
        res.name = "Flare";
        res.description = "Emits brilliant light for 8s; repels Weeper & Choir.";
    } else if (recipe.result == ItemType::NursesKit) {
        res.name = "Nurse's Kit";
        res.description = "Restores health and cures limp.";
    }

    return addItem(res);
}

} // namespace hm
