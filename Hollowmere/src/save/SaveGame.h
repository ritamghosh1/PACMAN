#pragma once

#include "core/mathx.h"
#include "story/Choices.h"
#include <string>
#include <vector>

namespace hm {

struct SaveData {
    int version = 1;
    float playTimeSec = 0.0f;
    Vec3 playerPos{0, 0, 0};
    float playerYaw = 0.0f;
    float playerPitch = 0.0f;
    int healthState = 0;
    float sanity = 100.0f;
    float lanternOil = 100.0f;

    std::vector<int> inventoryItemTypes;
    std::vector<int> unlockedDoorIds;
    std::vector<int> solvedPuzzleIds;
    std::vector<int> collectedNoteIds;
    std::vector<int> discoveredFragmentIds;

    GameChoices choices;
};

class SaveGame {
public:
    static bool saveToFile(const std::string& filepath, const SaveData& data);
    static bool loadFromFile(const std::string& filepath, SaveData& outData);
};

} // namespace hm
