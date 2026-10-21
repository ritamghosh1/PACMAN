#include "World.h"
#include "core/Input.h"
#include "story/Subtitles.h"
#include "story/Notes.h"
#include <algorithm>
#include <cmath>

namespace hm {

World::World() = default;
World::~World() = default;

bool World::init(QOpenGLFunctions_3_3_Core* gl) {
    m_audio.init();

    // 1. Generate procedural manor maze
    if (!MapGen::generateManor(gl, m_mapData)) {
        return false;
    }

    // 2. Initialize NavGrid from collision geometry
    m_navGrid.init(50, 50, 0.6f, Vec3(-15.0f, 0.0f, -15.0f));
    m_navGrid.buildFromCollision(m_mapData.collision, 0.35f);

    // 3. Spawn Player at foyer start
    m_player.spawn(m_mapData.playerSpawnPos);

    // 4. Spawn All 5 Distinct Ghosts in their designated wings
    // Ghost #1: The Weeper in Wing 1 (Library)
    m_ghosts.push_back(std::make_shared<Weeper>(Vec3(8.0f, 0.0f, 12.0f)));

    // Ghost #2: The Hollow Man in Wing 2 (Conservatory)
    m_ghosts.push_back(std::make_shared<HollowMan>(Vec3(-6.0f, 0.0f, 10.0f)));

    // Ghost #3: The Choir in Wing 3 (Cellar)
    m_ghosts.push_back(std::make_shared<Choir>(Vec3(12.0f, 0.0f, -6.0f)));

    // Ghost #4: The Doll (Eli) in Wing 4 (Nursery/Attic)
    m_ghosts.push_back(std::make_shared<Doll>(Vec3(-8.0f, 0.0f, -8.0f)));

    // Ghost #5: The Warden (Dr. Crane) in Wing 5 (Chapel stalker)
    m_ghosts.push_back(std::make_shared<Warden>(Vec3(15.0f, 0.0f, 15.0f)));

    // 5. Register door portals for sound propagation
    std::vector<Portal> portals;
    for (const auto& door : m_mapData.doors) {
        portals.push_back({door.position(), door.isLocked() || !door.isOpen(), 1.2f});
    }
    m_audio.setPortals(portals);

    // 6. Give starting inventory items
    m_inventory.addItem({ItemType::Matches, "Sulfur Matches", "Strikes light", 5, false, -1});
    m_inventory.addItem({ItemType::Bandage, "Linen Bandage", "Stops bleeding", 1, false, -1});
    m_inventory.addItem({ItemType::OilFlask, "Kerosene Flask", "Refills lantern", 1, false, -1});

    // 7. Initial journal entry
    auto allNotes = NoteLibrary::createAllNotes();
    if (!allNotes.empty()) {
        m_journal.addNote(allNotes[0]);
    }

    return true;
}

void World::update(float dt) {
    m_elapsedTime += dt;
    Input& input = Input::instance();

    // If death sequence is playing, advance it and handle respawn
    if (m_deathSequence.isActive()) {
        m_deathSequence.update(dt);
        if (m_deathSequence.phase() == DeathPhase::WakeUpFadeIn && m_deathSequence.phaseTimer() < dt * 1.5f) {
            // Wake up in nearest safe room
            Vec3 safePos(2.0f, 0.1f, 2.0f); // Default Foyer safe room
            if (!m_safeRooms.rooms().empty()) {
                safePos = m_safeRooms.rooms()[0].center;
            }
            m_player.spawn(safePos);
            m_player.restoreHealth();
            m_player.restoreSanity(70.0f);
            m_sanity.setSanity(70.0f);

            // Whisper upon waking
            SubtitleSystem::instance().show("Nora", m_deathSequence.wakeWhisper(), 4.5f, QColor(220, 180, 180));
        }
        return;
    }

    // 1. Update Doors
    for (auto& door : m_mapData.doors) {
        door.update(dt);
    }

    // 2. Update Pickups & Player interaction
    for (auto& pickup : m_mapData.pickups) {
        pickup.update(dt);
    }

    // Check player verbs (Blink/eyes closed with Ctrl, Listen with Alt, Hum with H)
    bool eyesClosed = input.isKeyDown(Qt::Key_Control);
    bool isListening = input.isKeyDown(Qt::Key_Alt);
    bool isHumming = input.isKeyDown(Qt::Key_H);
    bool isHoldingBreath = (input.isKeyDown(Qt::Key_Shift) && input.isKeyDown(Qt::Key_C));

    // Check if player is currently in a safe room
    bool inSafe = m_safeRooms.isInSafeRoom(m_player.position());

    // 3. Update Player
    m_player.update(dt, m_mapData.collision, m_mapData.doors, m_mapData.pickups);

    // Footstep audio trigger
    if (m_player.checkFootstepTriggered()) {
        m_audio.playFootstep(FootstepSurface::Wood, m_health.hasLimp(), m_player.isSprinting(), m_player.isCrouching());
    }

    // 4. Update Health & Sanity
    bool inDark = !m_player.lantern().isOn();
    m_health.update(dt, m_player.isSprinting());
    m_sanity.update(dt, inDark, m_fearLevel > 0.6f, inSafe, isListening, isHumming);
    m_player.restoreSanity(m_sanity.sanity() - m_player.sanity()); // keep in sync

    // 5. Update Mercy system
    if (isHumming && !m_mercy.isActive()) {
        m_mercy.startHum();
    }
    m_mercy.update(dt);

    // 6. Update Hallucinations
    m_hallucinations.update(dt, m_sanity.sanity(), m_player.position(), m_player.yaw());
    if (m_hallucinations.shouldTriggerPhantomStep()) {
        m_hallucinations.clearPhantomStepTrigger();
        m_audio.playFootstep(FootstepSurface::Stone, false, false, false);
    }

    // 7. Update Soft-Scare Triggers
    m_triggers.update(dt, m_player.position(), [this](TriggerType type, const ScareTrigger& t) {
        if (type == TriggerType::DoorSlam) {
            m_audio.playDoorSlam(t.position);
        } else if (type == TriggerType::LightFlicker) {
            m_audio.playFlicker();
        } else if (type == TriggerType::WhisperBehind) {
            m_audio.playWhisper(t.position);
        }
        if (!t.cueText.isEmpty()) {
            SubtitleSystem::instance().show("Manor", t.cueText, 3.5f, QColor(190, 190, 200));
        }
    });

    // 8. Update Ghosts (only if outside safe room)
    float minGhostDist = 999.0f;
    Vec3 pFwd(std::sin(radians(m_player.yaw())), 0.0f, std::cos(radians(m_player.yaw())));

    for (auto& ghost : m_ghosts) {
        ghost->update(dt,
                      m_player.position(),
                      pFwd,
                      m_player.lantern().isOn(),
                      m_player.lantern().isFocused(),
                      m_player.isSprinting(),
                      eyesClosed,
                      isHoldingBreath,
                      isHumming,
                      m_mapData.collision,
                      &m_navGrid);

        float d = ghost->proximityDistance();
        if (d < minGhostDist) minGhostDist = d;

        // Ghost attack trigger (blocked if player is inside safe room)
        if (ghost->hasAttacked() && !inSafe) {
            ghost->resetAttackFlag();
            m_player.takeDamage();
            m_health.takeDamage();
            m_audio.playJumpscareStinger();
            m_director.registerPlayerDeath();
            m_fearLevel = 1.0f;

            if (m_player.healthState() == HealthState::Critical || m_health.state() == PlayerHealthState::Dead) {
                m_deathSequence.trigger(ghost->position());
            }
        }

        // Positional audio triggers for signatures
        if (ghost->type() == GhostType::Weeper && d < 14.0f) {
            static float weeperSfx = 0.0f;
            weeperSfx += dt;
            if (weeperSfx > 3.5f) {
                weeperSfx = 0.0f;
                m_audio.playWeeperSob(ghost->position());
            }
        } else if (ghost->type() == GhostType::Choir && d < 16.0f) {
            static float choirSfx = 0.0f;
            choirSfx += dt;
            if (choirSfx > 4.5f) {
                choirSfx = 0.0f;
                m_audio.playChoirChord(ghost->position(), ghost->agitation() > 0.5f);
            }
        } else if (ghost->type() == GhostType::Warden && d < 20.0f) {
            static float wardenSfx = 0.0f;
            wardenSfx += dt;
            if (wardenSfx > 3.0f) {
                wardenSfx = 0.0f;
                m_audio.playWardenKeys(ghost->position());
            }
        }
    }

    // Determine active wing based on player Z position
    if (m_player.position().z < -5.0f) {
        m_activeWing = (m_player.position().x < 0.0f) ? 4 : 3; // Nursery or Cellar
    } else if (m_player.position().z > 12.0f) {
        m_activeWing = (m_player.position().x > 10.0f) ? 5 : ((m_player.position().x > 0.0f) ? 1 : 2);
    } else {
        m_activeWing = 0; // Foyer
    }

    // 9. Update Director and Fear level
    m_director.update(dt, m_player.sanity(), minGhostDist, inSafe);
    m_fearLevel = inSafe ? 0.0f : m_director.intensity();

    // 10. Update Audio Engine
    m_audio.setFearLevel(m_fearLevel);
    m_audio.setStamina(m_player.staminaRatio());
    m_audio.setInjured(m_health.isBleeding());
    m_audio.setHoldingBreath(isHoldingBreath);
    m_audio.setListening(isListening);
    m_audio.update(dt, m_player.eyePosition(), m_player.yaw());

    // 11. Update Subtitles
    SubtitleSystem::instance().update(dt);
}

} // namespace hm
