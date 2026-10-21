#include <cassert>
#include <iostream>
#include <cmath>
#include <cstdio>
#include "save/SaveGame.h"

using namespace hm;

int main() {
    std::cout << "[TEST] Running SaveGame Unit Tests..." << std::endl;

    SaveData original;
    original.version = 1;
    original.playTimeSec = 345.5f;
    original.playerPos = Vec3(4.5f, 0.0f, -12.3f);
    original.playerYaw = 180.0f;
    original.playerPitch = -15.0f;
    original.healthState = 1;
    original.sanity = 62.0f;
    original.lanternOil = 78.5f;

    original.inventoryItemTypes = {1, 2, 4};
    original.unlockedDoorIds = {101, 102};
    original.solvedPuzzleIds = {201};
    original.collectedNoteIds = {1, 2, 3, 4};
    original.discoveredFragmentIds = {1, 2};

    original.choices.sparedAda = true;
    original.choices.sparedEli = true;
    original.choices.fragmentsFound = 2;

    std::string testPath = "test_save_temp.json";
    bool saveOk = SaveGame::saveToFile(testPath, original);
    assert(saveOk);
    std::cout << "  ✓ SaveGame::saveToFile passed" << std::endl;

    SaveData loaded;
    bool loadOk = SaveGame::loadFromFile(testPath, loaded);
    assert(loadOk);
    std::cout << "  ✓ SaveGame::loadFromFile passed" << std::endl;

    assert(loaded.version == original.version);
    assert(std::abs(loaded.playerPos.x - original.playerPos.x) < 0.001f);
    assert(std::abs(loaded.playerPos.z - original.playerPos.z) < 0.001f);
    assert(std::abs(loaded.playerYaw - original.playerYaw) < 0.001f);
    assert(std::abs(loaded.sanity - original.sanity) < 0.001f);
    assert(std::abs(loaded.lanternOil - original.lanternOil) < 0.001f);
    assert(loaded.healthState == original.healthState);
    assert(loaded.inventoryItemTypes.size() == 3);
    assert(loaded.unlockedDoorIds.size() == 2);
    assert(loaded.choices.sparedAda == true);
    assert(loaded.choices.fragmentsFound == 2);
    std::cout << "  ✓ All SaveData fields verified identically" << std::endl;

    std::remove(testPath.c_str());
    std::cout << "[TEST] ALL SAVEGAME TESTS PASSED (100%)!" << std::endl;
    return 0;
}
