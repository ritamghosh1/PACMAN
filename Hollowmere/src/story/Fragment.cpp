#include "Fragment.h"

namespace hm {

MemorySystem::MemorySystem() {
    initDefaultFragments();
}

void MemorySystem::initDefaultFragments() {
    m_fragments.clear();
    m_fragments.push_back({
        1, "The Red Shoe", "Foyer — Under Grand Staircase",
        "A small red shoe left behind in the rain. 'Nora, don't leave me with the doctor.' The storm was cold that November night.",
        Vec3(1.5f, 0.0f, 3.5f), 1.5f, false
    });
    m_fragments.push_back({
        2, "The Silk Ribbon", "Library — Forbidden Aisle",
        "A blue ribbon woven between medical ledgers. Mae used to hide here, reading poetry under her wool shawl.",
        Vec3(6.5f, 0.0f, 10.5f), 1.5f, false
    });
    m_fragments.push_back({
        3, "Burnt Evaluation", "Library — Archives Vault",
        "Dr. Crane's handwriting: 'The subject demonstrates unusual auditory resonance. We must excise the attachment.'",
        Vec3(10.0f, 0.0f, 14.0f), 1.5f, false
    });
    m_fragments.push_back({
        4, "The Glass Bell", "Conservatory — Central Rotunda",
        "Shards of glass among overgrown ferns. Ada wept until her throat bled when they locked her in the cold dome.",
        Vec3(-5.5f, 0.0f, 8.5f), 1.5f, false
    });
    m_fragments.push_back({
        5, "The Pruning Shears", "Conservatory — Tool Shed",
        "Heavy blackened shears. Mr. Pell never spoke, only snipped the dead stems... and anything else that made a sound.",
        Vec3(-9.0f, 0.0f, 12.0f), 1.5f, false
    });
    m_fragments.push_back({
        6, "The Twine Wheel", "Cellar — Boiler Junction",
        "A valve handle bound with coarse twine. The heat was unbearable down here; the pipes screamed when pressure peaked.",
        Vec3(11.0f, 0.0f, -4.5f), 1.5f, false
    });
    m_fragments.push_back({
        7, "Tally of Silence", "Cellar — Sub-Level Cells",
        "Thirty-four scratches in the weeping mortar behind the steam pipe. 'I will be good today.'",
        Vec3(14.5f, 0.0f, -8.0f), 1.5f, false
    });
    m_fragments.push_back({
        8, "The Music Box", "Nursery — Playroom Hearth",
        "A tiny clockwork cylinder playing 'Hush Little Light'. Eli wound it whenever the night nurses came.",
        Vec3(-7.0f, 0.0f, -6.5f), 1.5f, false
    });
    m_fragments.push_back({
        9, "Stitched Eye Mask", "Nursery — Attic Rafters",
        "Soft burlap stitched with coarse black thread. 'If you cannot see them, they cannot find you.'",
        Vec3(-10.5f, 0.0f, -10.0f), 1.5f, false
    });
    m_fragments.push_back({
        10, "The Scorched Hymnal", "Chapel — Choir Pews",
        "Hymn 42 open on the charred cedar lectern. The notes were crossed out and replaced with medical dosage charts.",
        Vec3(12.5f, 0.0f, 12.5f), 1.5f, false
    });
    m_fragments.push_back({
        11, "The Warden's Ledger", "Chapel — Altar Crypt",
        "Dr. Crane's final confession: 'I did not want to cure them. I wanted to build an echo that would outlive grief.'",
        Vec3(16.0f, 0.0f, 16.0f), 1.5f, false
    });
    m_fragments.push_back({
        12, "Nurse Nora's Badge", "The Outer Gate — Threshold",
        "A brass badge tarnished green: 'Nora Crane, R.N. — Head Nurse'. You never came to Hollowmere to investigate. You came to remember.",
        Vec3(0.0f, 0.0f, -14.0f), 1.8f, false
    });
}

bool MemorySystem::checkTrigger(const Vec3& playerPos, MemoryFragment& outFragment) {
    for (auto& frag : m_fragments) {
        if (!frag.discovered && distance(playerPos, frag.triggerPos) <= frag.triggerRadius) {
            frag.discovered = true;
            m_discoveredCount++;
            outFragment = frag;
            return true;
        }
    }
    return false;
}

} // namespace hm
