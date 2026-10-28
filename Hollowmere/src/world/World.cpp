#include "World.h"
#include "core/Input.h"
#include "story/Subtitles.h"
#include "story/Notes.h"
#include "ai/Senses.h"
#include "world/wings/Wings.h"
#include "audio/CineSfx.h"
#include <algorithm>
#include <cmath>

namespace hm {

namespace {

const char* kWingShort[6] = {"foyer", "library", "conservatory", "boiler cellar", "attic nursery", "chapel"};

// Who "speaks" a found note, so the player hears the right typewriter timbre and motif.
Speaker noteSpeaker(int id) {
    switch (id) {
    case 6: case 14: case 22: case 29: case 36: case 38: case 40: return Speaker::Crane;
    case 10: case 12: case 13: case 30: return Speaker::Ada;
    case 31: case 32: case 35: return Speaker::Eli;
    case 3: case 20: case 39: return Speaker::Nora;
    default: return Speaker::Narrator;
    }
}

} // namespace

World::World() = default;
World::~World() = default;

bool World::init(QOpenGLFunctions_3_3_Core* gl) {
    m_gl = gl;
    if (!m_audioReady) {
        m_audio.init();
        m_audioReady = true;
    }
    buildLevel();
    return true;
}

void World::buildLevel() {
    m_mapData.wallMesh.destroy(m_gl);
    m_mapData.floorMesh.destroy(m_gl);
    m_mapData.ceilingMesh.destroy(m_gl);
    m_mapData.propsMesh.destroy(m_gl);
    MapGen::generateManor(m_gl, m_mapData);

    // Navigation grid covers the whole manor
    float span = m_mapData.gridW * m_mapData.cellSize;
    int cells = static_cast<int>(std::ceil(span / 0.6f));
    m_navGrid.init(cells, cells, 0.6f, m_mapData.origin);
    m_navGrid.buildFromCollision(m_mapData.collision, 0.35f);

    m_player = Player();
    m_player.spawn(m_mapData.playerSpawnPos, -90.0f);

    // One ghost per wing, each at home in its own maze
    m_ghosts.clear();
    auto& wi = m_mapData.wings;
    auto weeper = std::make_shared<Weeper>(wi[1].ghostSpawn);
    weeper->setPatrolRoute(wi[1].patrol);
    auto hollow = std::make_shared<HollowMan>(wi[2].ghostSpawn);
    auto choir = std::make_shared<Choir>(wi[3].ghostSpawn);
    choir->setPatrolRoute(wi[3].patrol);
    auto doll = std::make_shared<Doll>(wi[4].ghostSpawn);
    auto warden = std::make_shared<Warden>(wi[5].ghostSpawn);
    m_ghosts = {weeper, hollow, choir, doll, warden};
    m_ghostWasActive.assign(m_ghosts.size(), false);
    m_ghostBanishCounted.assign(m_ghosts.size(), false);

    // Sound propagation: doors are portals
    std::vector<Portal> portals;
    for (const auto& door : m_mapData.doors) portals.push_back({door.position(), door.isLocked() || !door.isOpen(), 1.2f});
    m_audio.setPortals(portals);

    // Safe rooms: the foyer and the candle-lit entrance of every wing
    std::vector<SafeRoomZone> zones;
    for (const auto& w : wi) zones.push_back({w.id, w.name.toStdString(), w.safeCenter, w.safeRadius, w.id == 0});
    m_safeRooms.setZones(zones);

    m_inventory = Inventory();
    m_health = HealthSystem();
    m_sanity = SanitySystem();
    m_hallucinations = HallucinationSystem();
    m_triggers = TriggerSystem();
    m_deathSequence = DeathSequence();
    m_mercy = MercySystem();
    m_choices = GameChoices();
    m_journal = Journal();
    m_director = Director();
    m_story.clear();
    m_explored.assign(m_mapData.gridW * m_mapData.gridH, false);
    ++m_exploredVersion;
    m_visitedMask = 1;
    m_activeWing = 0;
    m_pendingWingBanner = 0;
    m_wardenActive = false;
    m_endingRequested = false;
    m_pendingMemory = 0;
    m_fearLevel = 0.0f;
    m_elapsedTime = 0.0f;
    for (float& t : m_ghostSfxTimer) t = 0.0f;
    updateObjective();
}

void World::newGame() { buildLevel(); queueIntroBriefing(); }

void World::queueIntroBriefing() {
    queueStory(Speaker::Nora, "The gate sealed itself behind me. Mae is somewhere in this house.");
    queueStory(Speaker::Narrator, "This is St. Aldous Home, burned in 1998. Nora was a patient here as a child. The black-sealed letter says the dead children have taken her daughter.");
    queueStory(Speaker::Nora, "Each wing keeps one memory sigil: a piece of what happened that night. If I carry all four to the chapel altar, they might give her back.");
    queueStory(Speaker::Manor, "Move with W A S D and look with the mouse. Hold SHIFT to run, C to creep. F switches your lantern on and off.");
    queueStory(Speaker::Manor, "Press E to open doors and pick things up. TAB opens your journal and M toggles the map.");
    queueStory(Speaker::Nora, "The library is west of the foyer. Start there. Follow the gold arrow.");
    queueStory(Speaker::Manor, "Candle-lit rooms are safe. Whatever walks these halls cannot hurt you inside the light.");
}

int World::ghostWing(GhostType t) const {
    switch (t) {
    case GhostType::Weeper: return 1;
    case GhostType::HollowMan: return 2;
    case GhostType::Choir: return 3;
    case GhostType::Doll: return 4;
    case GhostType::Warden: return 5;
    }
    return 0;
}

void World::onWingEntered(int wing) {
    if (wing < 0 || wing > 5) return;
    m_pendingWingBanner = wing;
    m_audio.setWingAcoustics(WingsManager::getWing(wing).acousticType);
    if (m_visitedMask & (1 << wing)) return;
    m_visitedMask |= (1 << wing);
    switch (wing) {
    case 1:
        queueStory(Speaker::Nora, "Admission files. Every drawer is full of children's names.");
        queueStory(Speaker::Manor, "Something is weeping between the shelves. Creep (C) to stay unheard, or hold H near her to hum the lullaby.");
        break;
    case 2:
        queueStory(Speaker::Nora, "Glass domes and flowerbeds. Each bed has a small marker. These are graves.");
        queueStory(Speaker::Manor, "Mr. Pell only moves when you cannot see him. Keep your lantern lit and keep him in view.");
        break;
    case 3:
        queueStory(Speaker::Nora, "Steam and black water. And that sound in the walls... three voices as one.");
        queueStory(Speaker::Manor, "The Choir is blind. Walk, don't run. If they come close, hold your breath with SHIFT and C.");
        break;
    case 4:
        queueStory(Speaker::Nora, "Eli's nursery. Please, not here.");
        queueStory(Speaker::Eli, "Peek-a-boo… ready or not.");
        queueStory(Speaker::Manor, "Eli only wants the game. Hold CTRL to close your eyes while he is near, then open them. Do not hurt him.");
        break;
    case 5:
        queueStory(Speaker::Nora, "Mae? Mae, are you here?");
        queueStory(Speaker::Manor, "The altar has four hollows. Stand before it with all four sigils and press E.");
        break;
    default: break;
    }
}

int World::wingJustEntered() {
    int w = m_pendingWingBanner;
    m_pendingWingBanner = -1;
    return w;
}

void World::handlePickupEvents() {
    for (const auto& ev : m_player.takePickupEvents()) {
        m_audio.playPickupChime();
        switch (ev.type) {
        case PickupType::MemorySigil: {
            m_pendingMemory = std::clamp(ev.tag, 1, 4);   // GLView plays the animated flashback for this sigil
            if (m_player.sigilCount() >= 3 && !m_wardenActive) {
                m_wardenActive = true;
                m_ghosts[4]->resetToHome(m_mapData.wardenSpawn);
                queueStory(Speaker::Crane, "YOU HAVE TAKEN WHAT IS MINE, MISS HALE.");
                queueStory(Speaker::Manor, "Keys jingle somewhere in the dark. The Warden now walks the whole house. Run (SHIFT), or stun him with the focused lantern (hold right mouse).");
            }
            break;
        }
        case PickupType::Key:
            queueStory(Speaker::Narrator, ev.description);
            queueStory(Speaker::Manor, QString("The %1 opens the next door. Check your objective.").arg(ev.name));
            break;
        case PickupType::Note: {
            NoteData nd;
            for (const auto& n : NoteLibrary::createAllNotes()) if (n.id == ev.tag) nd = n;
            m_journal.addNote(nd);
            m_choices.fragmentsFound = m_journal.totalNotesCollected();
            if (ev.tag == 39) m_choices.confessedEarly = true;
            queueStory(noteSpeaker(ev.tag), ev.description);
            break;
        }
        case PickupType::Bandage:
            m_health = HealthSystem();
            break;
        case PickupType::OilFlask:
            break;
        }
    }
}

void World::handleDoorEvents() {
    const auto& bank = cinesfx::Bank::get();
    for (const auto& ev : m_player.takeDoorEvents()) {
        switch (ev.kind) {
        case DoorEvent::Opened: m_audio.playDoorCreak(ev.position); break;
        case DoorEvent::Closed: m_audio.playDoorCreak(ev.position); break;
        case DoorEvent::Unlocked: m_audio.playCine(bank.lock, 0.9f); m_audio.playDoorCreak(ev.position); break;
        case DoorEvent::Locked: m_audio.playCine(bank.lock, 0.7f); break;
        }
    }
    // Keep sound propagation in sync with door states
    std::vector<Portal> portals;
    for (const auto& door : m_mapData.doors) portals.push_back({door.position(), door.isLocked() || !door.isOpen(), 1.2f});
    m_audio.setPortals(portals);
}

void World::updateAltar() {
    m_extraPrompt.clear();
    Vec3 to = m_mapData.altarPos - m_player.position();
    to.y = 0;
    float d = length(to);
    if (d > 3.8f) return;
    if (dot(to * (1.0f / std::max(0.01f, d)), m_player.forward()) < 0.2f) return;
    bool ready = m_player.sigilCount() >= 4;
    m_extraPrompt = ready ? "[E] Place the four sigils on the altar" : QString("[E] Examine the altar (%1 of 4 sigils)").arg(m_player.sigilCount());
    Input& in = Input::instance();
    if (in.wasKeyPressed(Qt::Key_E) || in.wasMouseButtonPressed(Qt::LeftButton)) {
        if (ready) {
            m_endingRequested = true;
        } else {
            queueStory(Speaker::Narrator, QString("Four hollows are carved into the stone. You carry %1 sigil%2. Look in the other wings.").arg(m_player.sigilCount()).arg(m_player.sigilCount() == 1 ? "" : "s"));
        }
    }
}

void World::updateObjective() {
    ObjectiveInfo o;
    o.total = static_cast<int>(m_mapData.questOrder.size());
    const Pickup* next = nullptr;
    for (int id : m_mapData.questOrder) {
        const Pickup* p = m_mapData.findPickup(id);
        if (p && p->isCollected()) o.collected++;
        else if (p && !next) next = p;
    }
    if (next) {
        int wing = m_mapData.wingAt(next->position());
        const Door* door = wing > 0 ? m_mapData.findDoor(m_mapData.wings[wing].doorId) : nullptr;
        if (door && !door->isOpen()) {
            o.target = door->position();
            o.hasTarget = true;
            if (door->isLocked()) {
                o.title = QString("Unlock the %1 door").arg(kWingShort[wing]);
                o.detail = QString("Use the %1 on the locked door.").arg(Door::keyName(door->keyId()));
            } else {
                o.title = QString("Enter the %1").arg(kWingShort[wing]);
                o.detail = "Open the door and step inside.";
            }
        } else {
            o.target = next->position();
            o.hasTarget = true;
            if (next->type() == PickupType::MemorySigil) {
                o.title = QString("Find %1").arg(next->name());
                static const char* why[5] = {"", "Ada's memory: the girl silenced for crying. The Weeper haunts this maze.", "The garden's memory: small graves. Mr. Pell, the Hollow Man, tends them.", "The Chorus Ward's memory. The blind Choir listens for footsteps.", "Eli's memory: your brother and the door you locked. He plays hide-and-seek here."};
                o.detail = QString::fromUtf8(why[std::clamp(next->tag(), 1, 4)]);
            } else {
                o.title = QString("Find the %1").arg(next->name());
                o.detail = QString("It is somewhere in the %1 and opens the next wing.").arg(kWingShort[std::max(0, wing)]);
            }
        }
    } else {
        const Door* chapel = m_mapData.findDoor(m_mapData.wings[5].doorId);
        if (chapel && !chapel->isOpen()) {
            o.target = chapel->position();
            o.hasTarget = true;
            o.title = chapel->isLocked() ? "Unlock the chapel doors" : "Enter the chapel";
            o.detail = "The altar waits beyond. The Warden is awake.";
        } else {
            o.target = m_mapData.altarPos;
            o.hasTarget = true;
            o.title = "Place the four sigils on the altar";
            o.detail = "Stand before the altar and press E.";
        }
    }
    m_objective = o;
}

void World::updateGhosts(float dt, bool eyesClosed, bool holdingBreath, bool humming, bool inSafe) {
    const Vec3 pFwd = m_player.forward();
    float minGhostDist = 999.0f;
    float chaseDist = 999.0f;
    int chaseGhost = -1;
    int playerWing = m_mapData.wingAt(m_player.position());
    m_mapData.collision.setIgnoreTag(1);   // doors do not stop the dead
    // The Warden always knows roughly where you are, unless you are in a candle-lit refuge
    m_wardenPing += dt;
    if (m_wardenActive && !inSafe && m_wardenPing > 14.0f) {
        m_wardenPing = 0.0f;
        NoiseBus::instance().emitNoise(m_player.position(), 70.0f, 1.0f);
    }

    for (size_t i = 0; i < m_ghosts.size(); ++i) {
        auto& ghost = m_ghosts[i];
        bool active = (ghost->type() == GhostType::Warden) ? m_wardenActive : (playerWing == ghostWing(ghost->type()));
        if (!active) {
            if (m_ghostWasActive[i] && ghost->type() != GhostType::Warden) ghost->resetToHome(m_mapData.wings[ghostWing(ghost->type())].ghostSpawn);
            m_ghostWasActive[i] = false;
            ghost->setDormant();
            continue;
        }
        m_ghostWasActive[i] = true;
        ghost->update(dt, m_player.position(), pFwd, m_player.lantern().isOn(), m_player.lantern().isFocused(),
                      m_player.isSprinting(), eyesClosed, holdingBreath, humming, m_mapData.collision, &m_navGrid);

        float d = ghost->proximityDistance();
        if (!inSafe) minGhostDist = std::min(minGhostDist, d);
        if (!inSafe && ghost->state() == GhostState::Hunting && d < 14.0f && d < chaseDist) { chaseDist = d; chaseGhost = static_cast<int>(ghost->type()); }

        if (ghost->hasAttacked()) {
            ghost->resetAttackFlag();
            if (!inSafe) {
                m_player.takeDamage();
                m_health.takeDamage();
                m_audio.playJumpscareStinger();
                if (ghost->type() == GhostType::Weeper) m_audio.playClip(AssetAudio::instance().find("weeper_scream"), 0.9f);
                else if (ghost->type() == GhostType::Doll) m_audio.playClip(AssetAudio::instance().find("lure_idle6"), 0.9f);
                m_director.registerPlayerDeath();
                m_fearLevel = 1.0f;
                if (m_player.healthState() == HealthState::Critical || m_health.state() == PlayerHealthState::Dead)
                    m_deathSequence.trigger(ghost->position());
            }
        }

        // Spare / banish bookkeeping for the endings
        if (ghost->type() == GhostType::Weeper && std::dynamic_pointer_cast<Weeper>(ghost)->wasPacifiedByHum()) m_choices.sparedAda = true;
        if (ghost->type() == GhostType::Doll && ghost->state() == GhostState::Pacified) m_choices.sparedEli = true;
        if (ghost->type() == GhostType::Choir && std::dynamic_pointer_cast<Choir>(ghost)->isSingingPeacefully()) m_choices.sparedChoir = true;
        if ((ghost->state() == GhostState::Fleeing || ghost->state() == GhostState::Stunned) && !m_ghostBanishCounted[i]) {
            m_ghostBanishCounted[i] = true;
            m_choices.ghostsKilledOrBanished++;
        }

        // Each apparition announces itself with its own sound
        float& tmr = m_ghostSfxTimer[i];
        tmr += dt;
        if (ghost->type() == GhostType::Weeper && d < 16.0f && tmr > 3.5f) { tmr = 0; m_audio.playWeeperSob(ghost->position()); }
        else if (ghost->type() == GhostType::HollowMan && d < 14.0f && tmr > 2.2f) { tmr = 0; m_audio.playHollowManSnip(ghost->position()); }
        else if (ghost->type() == GhostType::Choir && d < 18.0f && tmr > 4.5f) { tmr = 0; m_audio.playChoirChord(ghost->position(), ghost->agitation() > 0.5f); }
        else if (ghost->type() == GhostType::Doll && d < 14.0f && tmr > 3.0f) { tmr = 0; m_audio.playDollGiggle(ghost->position()); }
        else if (ghost->type() == GhostType::Warden && d < 24.0f && tmr > 2.6f) { tmr = 0; m_audio.playWardenKeys(ghost->position()); }
    }
    m_mapData.collision.setIgnoreTag(-1);
    m_audio.setExplorationContext(m_activeWing, inSafe, chaseGhost);
    m_director.update(dt, m_player.sanity(), minGhostDist, inSafe);
    m_fearLevel = inSafe ? 0.0f : std::max(m_director.intensity(), minGhostDist < 12.0f ? (1.0f - minGhostDist / 12.0f) * 0.9f : 0.0f);
}

void World::update(float dt) {
    m_elapsedTime += dt;
    Input& input = Input::instance();
    NoiseBus::instance().update(dt);

    if (m_deathSequence.isActive()) {
        m_deathSequence.update(dt);
        if (m_deathSequence.phase() == DeathPhase::WakeUpFadeIn && m_deathSequence.phaseTimer() < dt * 1.5f) {
            // Wake in the nearest candle-lit refuge, fully healed
            Vec3 safePos = m_safeRooms.rooms().front().center;
            float best = 1e9f;
            for (const auto& r : m_safeRooms.rooms()) {
                float d = distance(m_player.position(), r.center);
                if (d < best) { best = d; safePos = r.center; }
            }
            m_player.spawn(safePos + Vec3(0, 0.1f, 0), m_player.yaw());
            m_player.restoreHealth(); m_player.restoreHealth();
            m_health = HealthSystem();
            m_sanity.setSanity(70.0f);
            m_player.refillOil(25.0f);
            for (auto& g : m_ghosts) if (g->type() != GhostType::Warden) g->resetToHome(m_mapData.wings[ghostWing(g->type())].ghostSpawn);
            if (m_wardenActive) m_ghosts[4]->resetToHome(m_mapData.wardenSpawn);
            SubtitleSystem::instance().show("Nora", m_deathSequence.wakeWhisper(), 4.5f, QColor(220, 180, 180));
        }
        SubtitleSystem::instance().update(dt);
        return;
    }

    for (auto& door : m_mapData.doors) door.update(dt);
    for (auto& pickup : m_mapData.pickups) pickup.update(dt);

    bool eyesClosed = input.isKeyDown(Qt::Key_Control);
    bool isListening = input.isKeyDown(Qt::Key_Alt);
    bool isHumming = input.isKeyDown(Qt::Key_H);
    bool isHoldingBreath = (input.isKeyDown(Qt::Key_Shift) && input.isKeyDown(Qt::Key_C));

    bool inSafe = m_safeRooms.isInSafeRoom(m_player.position());
    m_inSafe = inSafe;
    int playerWing = m_mapData.wingAt(m_player.position());

    m_player.update(dt, m_mapData.collision, m_mapData.doors, m_mapData.pickups);
    handlePickupEvents();
    handleDoorEvents();
    updateAltar();

    if (m_player.checkFootstepTriggered()) {
        m_audio.playFootstep(FootstepSurface::Wood, m_health.hasLimp(), m_player.isSprinting(), m_player.isCrouching());
        if (m_player.isSprinting()) NoiseBus::instance().emitNoise(m_player.position(), 6.0f, 0.8f);
    }

    bool inDark = !m_player.lantern().isOn();
    m_health.update(dt, m_player.isSprinting());
    m_sanity.update(dt, inDark && !inSafe, m_fearLevel > 0.6f, inSafe, isListening, isHumming);
    m_player.restoreSanity(m_sanity.sanity() - m_player.sanity());

    // Ada (the Weeper) can be calmed by humming in time with her sobs
    bool adaNear = playerWing == 1 && !m_mercy.isAdaPacified() && m_ghosts[0]->proximityDistance() < 9.0f;
    m_mercy.update(dt, isHumming, adaNear);
    if (adaNear && !isHumming && m_extraPrompt.isEmpty()) m_extraPrompt = "Ada is weeping nearby. Hold H to hum her lullaby";

    m_hallucinations.update(dt, m_sanity.sanity(), m_player.position(), m_player.yaw());
    if (m_hallucinations.shouldTriggerPhantomStep()) {
        m_hallucinations.clearPhantomStepTrigger();
        m_audio.playFootstep(FootstepSurface::Stone, false, false, false);
    }

    m_triggers.update(dt, m_player.position(), [this](TriggerType type, const ScareTrigger& t) {
        if (type == TriggerType::DoorSlam) m_audio.playDoorSlam(t.position);
        else if (type == TriggerType::LightFlicker) m_audio.playFlicker();
        else if (type == TriggerType::WhisperBehind) m_audio.playWhisper(t.position);
        if (!t.cueText.isEmpty()) SubtitleSystem::instance().show("Manor", t.cueText, 3.5f, QColor(190, 190, 200));
    });

    updateGhosts(dt, eyesClosed, isHoldingBreath, m_mercy.isAdaPacified(), inSafe);

    int w = m_mapData.wingAt(m_player.position());
    if (w >= 0 && w != m_activeWing) {
        m_activeWing = w;
        onWingEntered(w);
    }

    // Reveal the minimap around the player
    int pc, pr;
    if (m_mapData.worldToCell(m_player.position(), pc, pr)) {
        for (int dr = -3; dr <= 3; ++dr)
            for (int dc = -3; dc <= 3; ++dc) {
                int c = pc + dc, r = pr + dr;
                if (c >= 0 && c < m_mapData.gridW && r >= 0 && r < m_mapData.gridH && dc * dc + dr * dr <= 10 && !m_explored[r * m_mapData.gridW + c]) {
                    m_explored[r * m_mapData.gridW + c] = true;
                    ++m_exploredVersion;
                }
            }
    }
    updateObjective();

    m_audio.setFearLevel(m_fearLevel);
    m_audio.setStamina(m_player.staminaRatio());
    m_audio.setInjured(m_health.isBleeding());
    m_audio.setHoldingBreath(isHoldingBreath);
    m_audio.setListening(isListening);
    m_audio.update(dt, m_player.eyePosition(), m_player.yaw());

    SubtitleSystem::instance().update(dt);
}

SaveData World::makeSave() const {
    SaveData d;
    d.playerPos = m_player.position();
    d.playerYaw = m_player.yaw();
    d.playerPitch = m_player.pitch();
    d.lanternOil = m_player.lantern().oil();
    d.sanity = m_sanity.sanity();
    d.playTimeSec = m_elapsedTime;
    d.keyFlags = m_player.keyFlags();
    d.sigilCount = m_player.sigilCount();
    d.visitedWingMask = m_visitedMask;
    d.wardenActive = m_wardenActive;
    for (bool b : m_explored) d.exploredBits.push_back(b ? '1' : '0');
    d.choices = m_choices;
    for (const auto& p : m_mapData.pickups) if (p.isCollected()) d.collectedPickupIds.push_back(p.id());
    for (const auto& dr : m_mapData.doors) {
        if (!dr.isLocked()) d.unlockedDoorIds.push_back(dr.id());
        if (dr.isOpen()) d.openDoorIds.push_back(dr.id());
    }
    for (const auto& n : m_journal.allCollectedNotes()) d.collectedNoteIds.push_back(n.id);
    return d;
}

void World::applySave(const SaveData& d) {
    buildLevel();
    for (auto& p : m_mapData.pickups)
        if (std::find(d.collectedPickupIds.begin(), d.collectedPickupIds.end(), p.id()) != d.collectedPickupIds.end()) p.collect();
    for (auto& dr : m_mapData.doors) {
        bool open = std::find(d.openDoorIds.begin(), d.openDoorIds.end(), dr.id()) != d.openDoorIds.end();
        if (open) { dr.forceOpen(); m_mapData.collision.setColliderEnabled(dr.id(), false); }
        else if (std::find(d.unlockedDoorIds.begin(), d.unlockedDoorIds.end(), dr.id()) != d.unlockedDoorIds.end() && dr.keyId() != 0) {
            QString msg;
            dr.tryInteract(1 << dr.keyId(), msg);   // unlock...
            if (dr.isOpen()) dr.tryInteract(0, msg); // ...and leave it closed
        }
    }
    m_player.spawn(d.playerPos, d.playerYaw);
    m_player.setKeyFlags(d.keyFlags);
    m_player.setSigilCount(d.sigilCount);
    m_player.refillOil(d.lanternOil - 100.0f);
    m_sanity.setSanity(d.sanity);
    m_choices = d.choices;
    m_visitedMask = d.visitedWingMask | 1;
    m_wardenActive = d.wardenActive;
    if (m_wardenActive) m_ghosts[4]->resetToHome(m_mapData.wardenSpawn);
    const auto notes = NoteLibrary::createAllNotes();
    for (int id : d.collectedNoteIds) for (const auto& n : notes) if (n.id == id) m_journal.addNote(n);
    m_elapsedTime = d.playTimeSec;
    m_activeWing = std::max(0, m_mapData.wingAt(d.playerPos));
    m_pendingWingBanner = m_activeWing;
    for (size_t i = 0; i < d.exploredBits.size() && i < m_explored.size(); ++i) m_explored[i] = d.exploredBits[i] == '1';
    ++m_exploredVersion;
    updateObjective();
}

} // namespace hm

namespace hm {

QString World::storyRecap() const {
    QString s = "You are Nora Hale. Your daughter Mae has vanished, and a black-sealed letter led you to Hollowmere, the orphanage where you were a patient as a child. The dead children keep one memory sigil in each wing. Bring all four to the chapel altar.";
    int n = m_player.sigilCount();
    if (n >= 1) s += "\n\nAda, the Weeper, was silenced for crying. Her ribbon is yours now.";
    if (n >= 2) s += "\n\nMr. Pell \"pruned\" the noisy children in the garden. Their graves are marked with names.";
    if (n >= 3) s += "\n\nThe Chorus Ward: three children whose throats were joined. Dr. Crane, now the Warden, hunts you for taking what is his.";
    if (n >= 4) s += "\n\nEli, your little brother, is the Doll. You locked the ward door on him. The chapel is open: face the altar.";
    return s;
}

} // namespace hm
