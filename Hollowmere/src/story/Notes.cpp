#include "Notes.h"

namespace hm {

std::vector<NoteData> NoteLibrary::createAllNotes() {
    std::vector<NoteData> notes;
    notes.reserve(40);

    // Wing 0 — Foyer (1 to 6)
    notes.push_back({1, WingLore::Foyer, "The First Admittance", "November 12, 1928. The wards are full. The wind screams through the eaves like children at bath time.", Vec3(1, 0, 1), false});
    notes.push_back({2, WingLore::Foyer, "Rules of Conduct", "Rule IV: Lanterns must be extinguished by ten o'clock. What walks the corridors does not require light.", Vec3(3, 0, 2), false});
    notes.push_back({3, WingLore::Foyer, "Torn Letter", "Dearest Nora, you promised you would write when the clinic opened. The silence from the hills is terrifying.", Vec3(2, 0, 4), false});
    notes.push_back({4, WingLore::Foyer, "Delivery Manifest", "Six cases of formaldehyde, forty yards of coarse linen restraint webbing, one brass key to the east wing.", Vec3(4, 0, 3), false});
    notes.push_back({5, WingLore::Foyer, "Nurse's Log I", "Ada has not stopped weeping since her mother left. The sound carries down the laundry chutes.", Vec3(1, 0, 5), false});
    notes.push_back({6, WingLore::Foyer, "Dr. Crane's Directive", "Do not pacify their panic. Terror accelerates neural crystallization. We need pure emotion.", Vec3(5, 0, 4), false});

    // Wing 1 — Library (7 to 14)
    notes.push_back({7, WingLore::Library, "Index of Anatomical Plates", "Section III: Resonances of the human vocal fold under chemical stimulation.", Vec3(7, 0, 9), false});
    notes.push_back({8, WingLore::Library, "Cipher Key Part I", "The owl seeks the crescent moon; the serpent coils beneath the third ledger.", Vec3(8, 0, 11), false});
    notes.push_back({9, WingLore::Library, "Cipher Key Part II", "Four books bound in black goat leather. Reorder them by the dates of their deaths.", Vec3(9, 0, 12), false});
    notes.push_back({10, WingLore::Library, "Ada's Diary Page", "I hid behind the tall encyclopedias. If I hold my breath, the shadows glide past.", Vec3(10, 0, 10), false});
    notes.push_back({11, WingLore::Library, "Librarian's Warning", "Someone has been moving the heavy shelves at midnight. I heard dragging wood.", Vec3(6, 0, 13), false});
    notes.push_back({12, WingLore::Library, "Patient Chart 014", "Subject Ada. Diagnosis: Melancholia post-fever. Treatment: Cold immersion.", Vec3(11, 0, 11), false});
    notes.push_back({13, WingLore::Library, "Poetry Scrap", "Hush little light, don't say a word. Mama's gonna catch you a mockingbird.", Vec3(7, 0, 14), false});
    notes.push_back({14, WingLore::Library, "Crane Audio Log 02", "Transcript: 'Her tears are not saline. They leave crystalline tracks on the stone floor.'", Vec3(12, 0, 13), false});

    // Wing 2 — Conservatory (15 to 22)
    notes.push_back({15, WingLore::Conservatory, "Gardener's Ledger", "Mr. Pell keeps the glass spotless. He says dirt lets the light through unevenly.", Vec3(-4, 0, 7), false});
    notes.push_back({16, WingLore::Conservatory, "Mirror Alignment Note", "Rotate the mirrors clockwise until the beam catches the heart of the crystal iris.", Vec3(-6, 0, 9), false});
    notes.push_back({17, WingLore::Conservatory, "Botanical Study: Belladonna", "The roots thrive when watered with blood meal. The blooms open only in silence.", Vec3(-5, 0, 11), false});
    notes.push_back({18, WingLore::Conservatory, "Pell's Warning", "Do not look away from him once you enter the rose rotunda. He moves between heartbeats.", Vec3(-7, 0, 10), false});
    notes.push_back({19, WingLore::Conservatory, "Broken Glass Report", "A gust shattered the south dome. Two children fled into the thorn maze.", Vec3(-8, 0, 8), false});
    notes.push_back({20, WingLore::Conservatory, "Nurse Nora's Note", "I found Mae's ribbon caught on the pruning shears. Pell refused to answer me.", Vec3(-6, 0, 12), false});
    notes.push_back({21, WingLore::Conservatory, "Overgrowth Notice", "The vines have penetrated the drainage pipes. The walls are sweating sap.", Vec3(-9, 0, 11), false});
    notes.push_back({22, WingLore::Conservatory, "Crane Audio Log 04", "Transcript: 'Pell understands discipline. A plant must be cut down to bear fruit.'", Vec3(-7, 0, 13), false});

    // Wing 3 — Boiler Cellar (23 to 30)
    notes.push_back({23, WingLore::Cellar, "Boiler Pressure Chart", "Valves A and C must equal 140 PSI before turning the primary bypass lever.", Vec3(9, 0, -3), false});
    notes.push_back({24, WingLore::Cellar, "Flooding Warning", "Water level in the lower trench is ankle-deep. Watch for live wiring.", Vec3(11, 0, -5), false});
    notes.push_back({25, WingLore::Cellar, "The Chorus Ward", "Three children were brought down to the coal bunkers. Their voices merged into one continuous hum.", Vec3(12, 0, -7), false});
    notes.push_back({26, WingLore::Cellar, "Steam Valve Sequence", "When the red light flashes, vent 3 releases scalding steam for four seconds.", Vec3(10, 0, -8), false});
    notes.push_back({27, WingLore::Cellar, "Locker Assignment", "Locker 4 is reinforced. Can be locked from the inside with the brass latch.", Vec3(13, 0, -4), false});
    notes.push_back({28, WingLore::Cellar, "Faded Photograph", "Nora standing beside Dr. Crane in white surgical gowns. Both smiling.", Vec3(14, 0, -6), false});
    notes.push_back({29, WingLore::Cellar, "Crane Audio Log 07", "Transcript: 'The three throats vibrate at 220 Hertz. When they sing together, the mortar cracks.'", Vec3(11, 0, -9), false});
    notes.push_back({30, WingLore::Cellar, "Scribbled Prayer", "Lord let the water rise above our ears so we don't have to hear them singing.", Vec3(15, 0, -7), false});

    // Wing 4 — Nursery & Attic (31 to 36)
    notes.push_back({31, WingLore::Nursery, "Eli's Rules of the Game", "1. Count to ten. 2. Close your eyes. 3. If you open them too soon, Eli takes your eyes.", Vec3(-6, 0, -5), false});
    notes.push_back({32, WingLore::Nursery, "Music Box Note", "The third tooth on the brass cylinder is bent. It plays a flat fifth.", Vec3(-8, 0, -7), false});
    notes.push_back({33, WingLore::Nursery, "Toy Inventory", "Twelve porcelain dolls with genuine human hair. Three are missing from their cribs.", Vec3(-7, 0, -9), false});
    notes.push_back({34, WingLore::Nursery, "Night Matron's Confession", "The dolls walk at night. I found them arranged in a circle facing the wall.", Vec3(-9, 0, -6), false});
    notes.push_back({35, WingLore::Nursery, "Child's Drawing", "A tall man with iron keys and no face holding a little girl's hand in the dark.", Vec3(-10, 0, -8), false});
    notes.push_back({36, WingLore::Nursery, "Crane Audio Log 09", "Transcript: 'Eli believes this is all a game of hide and seek. I shall not disabuse him.'", Vec3(-8, 0, -11), false});

    // Wing 5 — Chapel & Finale (37 to 40)
    notes.push_back({37, WingLore::Chapel, "The Five Sigils", "Water, Bone, Glass, Iron, and Ash. Place them upon the altar to unseal the crypt.", Vec3(11, 0, 11), false});
    notes.push_back({38, WingLore::Chapel, "Dr. Crane's Testament", "I did not create the Hollow. Hollowmere was built upon a wound in the earth.", Vec3(13, 0, 13), false});
    notes.push_back({39, WingLore::Chapel, "Nora's Letter of Resignation", "I cannot do this anymore, Father. What you are doing to Mae is monstrous.", Vec3(15, 0, 14), false});
    notes.push_back({40, WingLore::Chapel, "The Warden's Final Order", "Seal all doors from the outside. No one leaves until the song is finished.", Vec3(16, 0, 16), false});

    return notes;
}

} // namespace hm
