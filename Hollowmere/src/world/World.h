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

namespace hm {

class World {
public:
    World();
    ~World();

    bool init(QOpenGLFunctions_3_3_Core* gl);
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
    DeathSequence& deathSequence() { return m_deathSequence; }
    MercySystem& mercy() { return m_mercy; }
    GameChoices& choices() { return m_choices; }
    Journal& journal() { return m_journal; }

    float fearLevel() const { return m_fearLevel; }
    float elapsedTime() const { return m_elapsedTime; }
    int activeWing() const { return m_activeWing; }

private:
    MapData m_mapData;
    NavGrid m_navGrid;
    Director m_director;
    Player m_player;
    std::vector<std::shared_ptr<Ghost>> m_ghosts;
    AudioEngine m_audio;

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

    int m_activeWing = 0;
    float m_fearLevel = 0.0f;
    float m_elapsedTime = 0.0f;
};

} // namespace hm
