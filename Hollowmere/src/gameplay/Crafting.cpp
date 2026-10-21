#include "Crafting.h"
#include <map>

namespace hm {

std::vector<CraftingRecipe> CraftingSystem::getRecipes() {
    return {
        // Flare: Bottle + Cloth + OilFlask
        {ItemType::Flare, 1, {ItemType::Bottle, ItemType::Cloth, ItemType::OilFlask}},
        // Nurse's Kit: Bandage + Painkiller
        {ItemType::NursesKit, 1, {ItemType::Bandage, ItemType::Painkiller}}
    };
}

bool CraftingSystem::canCraft(const std::vector<InventoryItem>& inventory, const CraftingRecipe& recipe) {
    std::map<ItemType, int> counts;
    for (const auto& item : inventory) {
        counts[item.type] += item.count;
    }

    for (ItemType req : recipe.ingredients) {
        if (counts[req] <= 0) return false;
        counts[req]--;
    }

    return true;
}

} // namespace hm
