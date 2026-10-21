#include "Difficulty.h"

namespace hm {

DifficultyTuning DifficultyTuning::get(DifficultyMode mode) {
    DifficultyTuning t;
    t.mode = mode;

    switch (mode) {
    case DifficultyMode::Explorer:
        t.ghostSpeedMultiplier = 0.70f;
        t.sanityDrainRate = 0.0f;
        t.oilBurnRate = 0.40f;
        t.playerDamageMultiplier = 0.50f;
        t.enableHud = true;
        t.infiniteSprint = true;
        t.savesPerWing = -1;
        break;

    case DifficultyMode::Story:
        t.ghostSpeedMultiplier = 1.0f;
        t.sanityDrainRate = 1.0f;
        t.oilBurnRate = 1.0f;
        t.playerDamageMultiplier = 1.0f;
        t.enableHud = true;
        t.infiniteSprint = false;
        t.savesPerWing = -1;
        break;

    case DifficultyMode::Survival:
        t.ghostSpeedMultiplier = 1.25f;
        t.sanityDrainRate = 1.6f;
        t.oilBurnRate = 1.5f;
        t.playerDamageMultiplier = 1.8f;
        t.enableHud = true;
        t.infiniteSprint = false;
        t.savesPerWing = 3;
        break;

    case DifficultyMode::Nightmare:
        t.ghostSpeedMultiplier = 1.45f;
        t.sanityDrainRate = 2.5f;
        t.oilBurnRate = 2.0f;
        t.playerDamageMultiplier = 3.0f;
        t.enableHud = false;
        t.infiniteSprint = false;
        t.savesPerWing = 1;
        break;
    }

    return t;
}

QString DifficultyTuning::name() const {
    switch (mode) {
    case DifficultyMode::Explorer: return "EXPLORER";
    case DifficultyMode::Story: return "STORY (RECOMMENDED)";
    case DifficultyMode::Survival: return "SURVIVAL";
    case DifficultyMode::Nightmare: return "NIGHTMARE";
    }
    return "UNKNOWN";
}

QString DifficultyTuning::description() const {
    switch (mode) {
    case DifficultyMode::Explorer:
        return "Focus on narrative and atmosphere. Slower ghosts, no sanity drain, infinite stamina.";
    case DifficultyMode::Story:
        return "The intended psychological horror experience. Balanced resources and standard ghost AI.";
    case DifficultyMode::Survival:
        return "Scarce lantern oil, faster and more relentless apparitions, limited safe room saves.";
    case DifficultyMode::Nightmare:
        return "No HUD or crosshair. 1 save per wing. Lethal apparitions. True psychological torment.";
    }
    return "";
}

} // namespace hm
