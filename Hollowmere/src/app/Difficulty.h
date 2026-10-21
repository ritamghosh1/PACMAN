#pragma once

#include <QString>

namespace hm {

enum class DifficultyMode {
    Explorer,
    Story,
    Survival,
    Nightmare
};

struct DifficultyTuning {
    DifficultyMode mode = DifficultyMode::Story;
    float ghostSpeedMultiplier = 1.0f;
    float sanityDrainRate = 1.0f;
    float oilBurnRate = 1.0f;
    float playerDamageMultiplier = 1.0f;
    bool enableHud = true;
    bool infiniteSprint = false;
    int savesPerWing = -1; // -1 = unlimited

    static DifficultyTuning get(DifficultyMode mode);
    QString name() const;
    QString description() const;
};

} // namespace hm
