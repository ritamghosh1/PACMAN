#pragma once

#include <vector>
#include <memory>
#include <QOpenGLFunctions_3_3_Core>
#include "MapGen.h"
#include "NavGrid.h"
#include "ai/Director.h"
#include "entities/Player.h"
#include "entities/Ghost.h"
#include "entities/Weeper.h"
#include "entities/HollowMan.h"
#include "entities/Choir.h"
#include "entities/Doll.h"
#include "entities/Warden.h"
#include "audio/AudioEngine.h"
#include "gameplay/Inventory.h"
#include "gameplay/Health.h"
#include "gameplay/Sanity.h"
#include "gameplay/Hallucinations.h"
#include "gameplay/Mercy.h"
#include "gameplay/Death.h"
#include "world/Triggers.h"
#include "world/SafeRoom.h"
#include "story/Choices.h"
#include "story/Journal.h"
#include "story/Dialogue.h"
#include "save/SaveGame.h"

namespace hm {

// What the player should do next, and where it is. Drives the HUD arrow and minimap marker.
struct ObjectiveInfo {
    QString title;
    QString detail;
    Vec3 target;
    bool hasTarget = false;
    int collected = 0;      // quest items collected so far (0..8)
    int total = 8;
};

class World {
public:
    World();
    ~World();

    bool init(QOpenGLFunctions_3_3_Core* gl);
    void newGame();                      // wipe progress and rebuild the manor
    void update(float dt);

    Player& player() { return m_player; }
    const Player& player() const { return m_player; }

    const MapData& map() const { return m_mapData; }
    MapData& map() { return m_mapData; }
    const std::vector<std::shared_ptr<Ghost>>& ghosts() const { return m_ghosts; }
    const NavGrid& navGrid() const { return m_navGrid; }
    Director& director() { return m_director; }

    AudioEngine& audio() { return m_audio; }

    Inventory& inventory() { return m_inventory; }
    const Inventory& inventory() const { return m_inventory; }
    HealthSystem& health() { return m_health; }
    const HealthSystem& health() const { return m_health; }
    SanitySystem& sanity() { return m_sanity; }
    const SanitySystem& sanity() const { return m_sanity; }
    HallucinationSystem& hallucinations() { return m_hallucinations; }
    const HallucinationSystem& hallucinations() const { return m_hallucinations; }

    TriggerSystem& triggers() { return m_triggers; }
    SafeRoomManager& safeRooms() { return m_safeRooms; }
    const SafeRoomManager& safeRooms() const { return m_safeRooms; }
    DeathSequence& deathSequence() { return m_deathSequence; }
    MercySystem& mercy() { return m_mercy; }
    GameChoices& choices() { return m_choices; }
    Journal& journal() { return m_journal; }

    float fearLevel() const { return m_fearLevel; }
    float elapsedTime() const { return m_elapsedTime; }
    int activeWing() const { return m_activeWing; }
    bool inSafeRoom() const { return m_inSafe; }

    // Story beats to be shown in the typewriter box (the sim pauses while they are on screen).
    std::vector<DialogueLine> takeStory() { auto s = std::move(m_story); m_story.clear(); return s; }
    void queueStory(Speaker who, const QString& text) { m_story.push_back({who, text}); }
    void queueIntroBriefing();

    const ObjectiveInfo& objective() const { return m_objective; }
    int takeMemory() { int m = m_pendingMemory; m_pendingMemory = 0; return m; }   // sigil number whose flashback should play (0 = none)
    QString storyRecap() const;
    QString interactionPrompt() const { return m_extraPrompt.isEmpty() ? m_player.currentPrompt() : m_extraPrompt; }
    const std::vector<bool>& explored() const { return m_explored; }
    int exploredVersion() const { return m_exploredVersion; }
    bool endingRequested() const { return m_endingRequested; }
    void clearEndingRequest() { m_endingRequested = false; }
    bool wardenActive() const { return m_wardenActive; }
    int wingJustEntered(); // returns wing id once per entry, else -1 (for the location banner)

    SaveData makeSave() const;
    void applySave(const SaveData& data);

private:
    void buildLevel();
    void handlePickupEvents();
    void handleDoorEvents();
    void updateGhosts(float dt, bool eyesClosed, bool holdingBreath, bool humming, bool inSafe);
    void updateAltar();
    void updateObjective();
    void onWingEntered(int wing);
    int ghostWing(GhostType t) const;

    QOpenGLFunctions_3_3_Core* m_gl = nullptr;
    MapData m_mapData;
    NavGrid m_navGrid;
    Director m_director;
    Player m_player;
    std::vector<std::shared_ptr<Ghost>> m_ghosts;
    std::vector<bool> m_ghostWasActive;
    std::vector<bool> m_ghostBanishCounted;
    AudioEngine m_audio;
    bool m_audioReady = false;

    Inventory m_inventory;
    HealthSystem m_health;
    SanitySystem m_sanity;
    HallucinationSystem m_hallucinations;
    TriggerSystem m_triggers;
    SafeRoomManager m_safeRooms;
    DeathSequence m_deathSequence;
    MercySystem m_mercy;
    GameChoices m_choices;
    Journal m_journal;

    std::vector<DialogueLine> m_story;
    ObjectiveInfo m_objective;
    QString m_extraPrompt;
    std::vector<bool> m_explored;
    int m_pendingMemory = 0;
    int m_visitedMask = 0;
    int m_exploredVersion = 0;
    int m_activeWing = 0;
    int m_pendingWingBanner = -1;
    bool m_inSafe = false;
    bool m_wardenActive = false;
    bool m_endingRequested = false;
    float m_fearLevel = 0.0f;
    float m_elapsedTime = 0.0f;
    float m_wardenPing = 0.0f;
    float m_ghostSfxTimer[5] = {0, 0, 0, 0, 0};
};

} // namespace hm
