#include "Wings.h"

namespace hm {

WingDefinition WingsManager::getWing(int id) {
    WingDefinition w;
    w.wingId = id;

    switch (id) {
        case 0:
            // Wing 0 — FOYER (Grand Hub & Tutorial)
            w.name = "The Grand Foyer";
            w.acousticType = WingAcousticType::StoneHall;
            w.musicState = MusicState::DreadBed;
            w.ambientLight = Vec3(0.09f, 0.08f, 0.12f);
            w.fogColor = Vec3(0.04f, 0.04f, 0.07f);
            w.fogDensity = 0.04f;
            w.hasSafeRoom = true;
            w.safeRoomCenter = Vec3(2.0f, 0.0f, 2.0f);
            break;

        case 1:
            // Wing 1 — LIBRARY (Bookshelves, Archive Vault, The Weeper)
            w.name = "The West Library";
            w.acousticType = WingAcousticType::WoodLibrary;
            w.musicState = MusicState::DreadBed;
            w.ambientLight = Vec3(0.10f, 0.07f, 0.04f);
            w.fogColor = Vec3(0.05f, 0.04f, 0.03f);
            w.fogDensity = 0.05f;
            w.hasSafeRoom = true;
            w.safeRoomCenter = Vec3(10.0f, 0.0f, 6.0f);
            break;

        case 2:
            // Wing 2 — CONSERVATORY (Glass Domes, Overgrowth, The Hollow Man)
            w.name = "The Green Conservatory";
            w.acousticType = WingAcousticType::GlassDome;
            w.musicState = MusicState::DreadBed;
            w.ambientLight = Vec3(0.04f, 0.10f, 0.06f);
            w.fogColor = Vec3(0.02f, 0.06f, 0.04f);
            w.fogDensity = 0.065f;
            w.hasSafeRoom = true;
            w.safeRoomCenter = Vec3(-8.0f, 0.0f, 6.0f);
            break;

        case 3:
            // Wing 3 — BOILER CELLAR (Flooded trenches, Steam pipes, The Choir)
            w.name = "The Boiler Cellar";
            w.acousticType = WingAcousticType::DampCellar;
            w.musicState = MusicState::Pursuit;
            w.ambientLight = Vec3(0.06f, 0.04f, 0.03f);
            w.fogColor = Vec3(0.07f, 0.05f, 0.04f);
            w.fogDensity = 0.08f;
            w.hasSafeRoom = true;
            w.safeRoomCenter = Vec3(8.0f, 0.0f, -8.0f);
            break;

        case 4:
            // Wing 4 — NURSERY & ATTIC (Cribs, Toys, The Doll Eli)
            w.name = "The Attic Nursery";
            w.acousticType = WingAcousticType::SoftNursery;
            w.musicState = MusicState::Emotional;
            w.ambientLight = Vec3(0.08f, 0.07f, 0.09f);
            w.fogColor = Vec3(0.06f, 0.05f, 0.07f);
            w.fogDensity = 0.055f;
            w.hasSafeRoom = true;
            w.safeRoomCenter = Vec3(-6.0f, 0.0f, -12.0f);
            break;

        case 5:
            // Wing 5 — CHAPEL (Finale, Altar, The Warden Dr. Crane)
            w.name = "The High Chapel";
            w.acousticType = WingAcousticType::Cathedral;
            w.musicState = MusicState::Pursuit;
            w.ambientLight = Vec3(0.06f, 0.02f, 0.02f);
            w.fogColor = Vec3(0.05f, 0.02f, 0.02f);
            w.fogDensity = 0.05f;
            w.hasSafeRoom = true;
            w.safeRoomCenter = Vec3(14.0f, 0.0f, 12.0f);
            break;
    }

    return w;
}

} // namespace hm
