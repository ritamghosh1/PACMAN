#pragma once

#include "Item.h"
#include <vector>

namespace hm {

struct CraftingRecipe {
    ItemType result;
    int resultCount = 1;
    std::vector<ItemType> ingredients;
};

class CraftingSystem {
public:
    static std::vector<CraftingRecipe> getRecipes();
    static bool canCraft(const std::vector<InventoryItem>& inventory, const CraftingRecipe& recipe);
};

} // namespace hm
