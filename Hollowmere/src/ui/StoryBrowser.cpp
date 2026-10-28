#include "StoryBrowser.h"
#include "FontCache.h"
#include "story/CineArt.h"
#include <QLinearGradient>
#include <QPainterPath>
#include <QRadialGradient>
#include <algorithm>
#include <cmath>

namespace hm {

using namespace cineart;

StoryBrowser::StoryBrowser() {
    m_pages = {
        {"THE STORY", "A psychological horror in a haunted orphanage",
         "Hollowmere is St. Aldous Home for Children, an orphanage and clinic that burned down on the night of 3 November 1998. It was paid for by the wealthy Hale family and run by Dr. Silas Crane.\n\n"
         "Nora Hale grew up inside it. Twenty-five years later she is a trauma nurse and a single mother. Her six-year-old daughter Mae disappears from school. That morning a music box arrives in the post, playing a lullaby Nora has hummed to Mae every night and never remembered learning. With it comes a letter sealed in black wax, addressed to the one place Nora has spent her whole adult life forgetting.\n\n"
         "The children who died there have not left. Each one is frozen in the way they were hurt, repeating the last night. They have taken Mae because they want a big sister, and the last big sister they had locked them in.\n\n"
         "To get Mae back Nora must walk through the burned house, recover four memory sigils (one small object from each wing, each a piece of what really happened), and carry them to the chapel altar. What she does to the ghosts on the way decides which of five endings she gets.\n\n"
         "Read the pages on the left in order for the full background: the Programme, Nora's past, the night of the fire, why Mae is involved, the ghosts, and every ending."},

        {"THE QUIET PROGRAMME", "What Dr. Crane did at St. Aldous",
         "St. Aldous Home was a charity orphanage and children's clinic, opened in 1991 and funded by the Hale family. Its director, Dr. Silas Crane, believed that grief is contagious: one crying child makes a ward of crying children, and crying delays \"recovery.\" His answer was a private research project he called THE QUIET PROGRAMME.\n\n"
         "Its principle was simple and monstrous: a child who cannot cry cannot suffer where anyone can see it. Over seven years Crane and his staff tested methods on twelve children, recorded in a ledger he kept locked in the chapel.\n\n"
         "WHAT IT DID TO THEM\n"
         "  - Sensory deprivation: children kept in dark rooms \"until the noise stopped.\"\n"
         "  - Sedation and restraint, with straps and a chair in the cellar ward.\n"
         "  - Silencing: Ada's tear ducts and voice were sealed because she cried at night.\n"
         "  - \"Pruning\": the orderly Mr. Pell punished noisy children in the greenhouse garden. Some never came back; their beds are marked with name tags.\n"
         "  - The Chorus Ward: three children had their throats surgically joined so that they could only produce one calm chord.\n"
         "  - Masks: Eli was made to wear a porcelain mask \"to calm him,\" and to keep his eyes closed.\n\n"
         "WHO KNEW\n"
         "The staff signed papers and stayed silent. The Hale family paid for it and asked for no reports. The children's only comfort was a lullaby the older ones taught the younger ones in whispers: \"Hush, Little Light.\"\n\n"
         "The Programme ended in a single night. See \"The Night of the Fire.\""},

        {"NORA'S PAST", "What it means that she was a patient",
         "Nora does not remember being at St. Aldous, but the house remembers her. She was not a visitor or a nurse there. She was Subject 9, admission file 404.\n\n"
         "At nine years old Nora and her five-year-old brother Eli were placed in the home by their mother. Eli had terrible night terrors, and Dr. Crane promised to cure him. A Hale child was a \"charity ward,\" which meant the family paid and the staff were told nothing about visits. Nora was kept as a helper and as a test case: Crane used her to see whether a calm child could keep other children calm.\n\n"
         "WHAT HAPPENED TO HER MEMORY\n"
         "After the fire Nora was found on the lawn, alive, and did not speak for a year. Doctors called it shock. In truth Crane's sedatives and the Programme's \"quiet hour\" had sealed the memory: a locked door in her mind, which is why locked doors are the game's central image. Her mother never mentioned St. Aldous again and died when Nora was twenty. The Hale name quietly buried the scandal.\n\n"
         "WHAT SHE BECAME\n"
         "Nora grew up to be a calm, clinical trauma nurse who cannot sleep, avoids fire, and hums a lullaby she cannot remember learning. She is calm around blood because she has already seen the worst. She is also terrified of closed doors.\n\n"
         "Being a patient means every ghost in the house knows her. They do not see an intruder; they see the girl who ran."},

        {"THE NIGHT OF THE FIRE", "3 November 1998",
         "The night was meant to be like any other quiet hour. Nora, nine, woke and found Eli's bed empty. She searched the house and found him in the cellar ward, strapped in the chair with his porcelain mask on, crying silently. The other children were in their cribs and cells, drugged and sealed.\n\n"
         "Nora tried to take them all out. Dr. Crane caught her at the nursery stairs. In the struggle a kerosene lamp fell, and the old wooden wing caught fire. The corridors filled with smoke.\n\n"
         "Nora reached the only exit: the heavy ward door. She could hear Crane behind her, coming for her, and Eli and the other children screaming inside. In a single terrified second she did the thing she has forgotten for twenty-five years. She slammed the door and turned the key to stop Crane following her.\n\n"
         "It stopped Crane. It also locked the children inside. Eli screamed her name through the wood. She ran.\n\n"
         "The fire killed Crane, Pell and eleven children. Crane's body fused with the burnt door of the ward. That is what the Warden is. Everything in Hollowmere is organised around the door Nora locked and the children behind it."},

        {"WHY MAE?", "How Mae got pulled into this",
         "Mae is Nora's daughter. She is six, the same age Eli was frozen at, and she is the one person Nora has told nothing about her past because Nora does not know it.\n\n"
         "WHY THE CHILDREN TOOK HER\n"
         "1. The lullaby. Nora has hummed \"Hush, Little Light\" to Mae at bedtime since she was a baby. The song is the one thing from St. Aldous that survived in Nora. The ghosts can hear it through the house's link to Nora's memory, and to them anyone who knows the song is family.\n"
         "2. A replacement sister. The children in the house are stuck repeating the last night. They want a big sister to stay with them, and Nora is the one who left. Mae, who sings their song, is the nearest thing.\n"
         "3. Bait. The only way to make Nora come back to the house is to take what she loves, the same way she took their sister away from them. They do not want to hurt Mae; they want Nora to answer for what she did.\n\n"
         "WHY CRANE (THE WARDEN) INTERFERES\n"
         "Crane's remains are trying to stop the children. The house exists only while the children are unforgiven and kept quiet. If Nora confesses and the children forgive her, the house ends and so does he. So the Warden hunts Nora once she holds the sigils.\n\n"
         "ANOTHER WAY TO READ IT\n"
         "Hollowmere may be Nora's own memory made flesh while she lies in a burns unit after a breakdown. In that reading Mae is safe in the real world with Nora's sister, and the whole game is Nora deciding whether she will finally open the door. The journal supports both readings."},

        {"THE FOUR SIGILS", "Why four objects open the chapel",
         "Each wing hides one memory sigil. Each is an object a child held on the last night, and carrying it triggers a flashback that explains who the ghost of that wing was.\n\n"
         "1. ADA'S CRIMSON RIBBON (West Library). Ada, eight, was silenced for crying. She tied the ribbon in her hair so Nora could find her in the dark.\n\n"
         "2. THE TOOTH RING (Green Conservatory). A child's tooth threaded on a ring, taken from a flowerbed that was really a grave. It proves what Mr. Pell \"pruned.\"\n\n"
         "3. THE BENT TUNING FORK (Boiler Cellar). The note the Chorus Ward was made to sing. It bends in your hand and hums.\n\n"
         "4. ELI'S BLANKET (Attic Nursery). Eli carried it everywhere. It still smells of smoke. It is the last sigil because it is the one Nora most wants to avoid.\n\n"
         "Each wing also hides a key that opens the next one. After the third sigil the Warden wakes. When Nora lays all four on the chapel altar, the four hollows in the stone fill and the ward door opens. What happens next depends on what she did to the ghosts."},

        {"THE GHOSTS", "Click a portrait to read their story", "", -1, true},

        {"HOW IT PLAYS", "First-person horror maze, no voice acting",
         "GOAL: find four memory sigils (one per wing), use the key found in each wing to open the next, then place the sigils on the chapel altar. A gold arrow, a marker and a minimap always point to your objective.\n\n"
         "EACH GHOST HAS THREE ANSWERS: avoid it, outsmart it, or show it mercy.\n"
         "  Ada the Weeper: creep (C) past her, burn her with the focused lantern (right mouse), or hold H and click on the beat to hum her lullaby.\n"
         "  Mr. Pell, the Hollow Man: he only moves when unseen. Keep your lantern on him.\n"
         "  The Choir: blind. Walk, never run; hold your breath (SHIFT+C).\n"
         "  Eli the Doll: close your eyes (CTRL) while he is near, then open them: peek-a-boo.\n"
         "  The Warden: runs once you hold three sigils. Sprint (SHIFT) or stun him with the focused lantern.\n\n"
         "Candle-lit rooms (the foyer and each wing entrance) are safe.\n\n"
         "SANITY (blue bar): darkness, ghosts and listening drain it; candlelight, a lit lantern and humming restore it. As it falls the colour drains, you see shadows, hear whispers and footsteps that are not there, and eventually see ghosts that are not real. It never kills you directly: it makes the house lie to you.\n\n"
         "THE STORY IS TOLD WITHOUT VOICES: typewriter text you click through, animated flashbacks after every sigil, and sound. Each character has a signature tick and melody so you recognise who is speaking by ear.\n\n"
         "SOUND: recordings of ambience, screams and music from the open-source game SCP: Containment Breach (CC BY-SA 3.0, credited in assets/audio/ATTRIBUTION.md), mixed with procedural sound. TECH: Qt 5 and OpenGL 3.3 core; geometry, textures and shaders are all generated in code."},

        {"ENDINGS OVERVIEW", "Five endings, decided by how you treat the ghosts",
         "Nora's choices are tracked all game. When she places the sigils on the altar, the game checks them in this order:\n\n"
         "  1. SECRET: you read all 16 wing notes and Nora's resignation letter  ->  Ending V\n"
         "  2. You spared all three kindred ghosts (Ada, Eli, the Choir) and read at least 8 notes  ->  Ending I\n"
         "  3. You spared all three but read fewer than 8 notes  ->  Ending IV\n"
         "  4. You drove off or stunned three or more ghosts  ->  Ending II\n"
         "  5. Anything else  ->  Ending III\n\n"
         "WHAT COUNTS AS MERCY\n"
         "  Ada: hum her lullaby in time (hold H, click on the green).\n"
         "  Eli: close your eyes near him, then open them (peek-a-boo).\n"
         "  The Choir: stand still and hold your breath near them until they sing in peace.\n\n"
         "WHAT COUNTS AS BANISHING\n"
         "  Stunning or repelling a ghost with the focused lantern.\n\n"
         "WHAT COUNTS AS KNOWLEDGE\n"
         "  Every note you read adds to the truth you carry to the altar. Reading enough lets Nora confess honestly. Reading everything shows her who she really was.\n\n"
         "Select Ending I to V on the left to read each one explained, and to watch it."},

        {"ENDING I", "THE DAWN AT HOLLOWMERE  (best ending)",
         "HOW TO GET IT: spare all three kindred ghosts and read at least 8 of the 20 notes.\n\n"
         "WHAT HAPPENS: The sigils settle into the altar and the ward door swings open. Nora says it aloud at last: she locked them in, and she is sorry. The children forgive her. The house burns away into morning, and when she wakes, Mae's red ribbon is warm in her hand.\n\n"
         "WHAT IT MEANS: Nora faces what she did instead of burying it. Mercy toward the ghosts was practice for mercy toward herself. Because she forgives the children and is forgiven in turn, the house has nothing left to hold, and Mae comes home. This is the only ending where everyone is freed.", 0},
        {"ENDING II", "THE NEW CARETAKER  (dark ending)",
         "HOW TO GET IT: drive off or stun three or more ghosts with the lantern instead of showing mercy.\n\n"
         "WHAT HAPPENS: Ash drifts through the foyer like black snow. The cold seeps into Nora until her breath no longer steams. She lifts the iron keys from the floor. Someone must mind the wards.\n\n"
         "WHAT IT MEANS: Nora answered fear with force, the same way Dr. Crane did. By forcing the children into silence she repeats the Programme and becomes the next Warden. The cycle of the house survives; only the keeper changes. Mae is never mentioned again.", 1},
        {"ENDING III", "THE ENDLESS DRIVE  (default ending)",
         "HOW TO GET IT: reach the altar without sparing everyone and without banishing three ghosts.\n\n"
         "WHAT HAPPENS: Nora drags Mae to the car and drives without looking back. The radio crackles into a music box, a lullaby in three-four time. In the mirror, the manor waits. The road remembers her.\n\n"
         "WHAT IT MEANS: She saved her daughter but not the children, and she did not face the door. Running away worked once in 1998 and it works again, but the story loops: the same road, the same lullaby, and Mae, silent in the passenger seat, who now carries it too. It is a rescue without a resolution.", 2},
        {"ENDING IV", "CHORUS OF THE INNOCENT  (bittersweet)",
         "HOW TO GET IT: spare all three ghosts but read fewer than 8 notes.\n\n"
         "WHAT HAPPENS: Nora kneels at the altar and sings the children to rest. Their small shapes dissolve into candlelight, and Mae walks out alone into the dawn. The house keeps Nora as its quiet guardian.\n\n"
         "WHAT IT MEANS: Nora is merciful, but she does not understand enough of the truth to forgive herself, so she makes a sacrifice instead of a confession. The children are free and Mae is safe, but Nora stays. It is atonement without understanding: generous, tragic and incomplete.", 3},
        {"ENDING V", "THE DOCTOR'S ARCHIVE  (secret ending)",
         "HOW TO GET IT: find and read all 16 wing notes plus Nora's resignation letter in the chapel.\n\n"
         "WHAT HAPPENS: The hidden drawer clicks open. Inside lies a yellowed medical chart in Dr. Crane's hand: Patient 404, Nora Hale. The truth washes over the shadows like daylight over broken glass.\n\n"
         "WHAT IT MEANS: The full record shows Nora was never an outsider. She was Subject 9, the Programme's test case, and her \"resignation letter\" was written while she was a child under sedation. This ending reframes the villain and the heroine, and supports the reading that the whole house is Nora's own memory.", 4},
    };
    m_ghosts = {
        {"ADA", "The Weeper", "Eight years old. Silenced for crying.",
         "Ada was the youngest girl in the library ward. She cried every night for her mother, who had left her at the home and never came back. Dr. Crane called her \"the noisiest child I have ever treated.\" The Programme sealed her tear ducts and her voice. She could still cry, but only blood-tears and no sound, so she began to cry silently at the library window.\n\n"
         "She tied a red ribbon in her hair so Nora, who was kind to her, could find her in the dark. On the night of the fire Ada was locked in the library with the others. She still wanders the shelves weeping.\n\n"
         "She is not hostile to Nora. She is afraid of the light and of loud footsteps, because both mean someone is coming to take her.",
         "AVOID: creep with C so she does not hear you.\nOUTSMART: hold the focused lantern (right mouse) on her until she flees.\nMERCY: hold H and click on the beat to hum Hush, Little Light. She calms and sits quietly.",
         "Her sobbing is a real recording of whimpering and sniffles; her attack scream is the sound of grief breaking."},
        {"MR. PELL", "The Hollow Man", "The gardener who \"pruned\" the children.",
         "Mr. Pell was an orderly and gardener, and the only adult on staff who enjoyed the Programme. He ran the greenhouse as a punishment room. A noisy child was sent to \"the garden\" and came back quiet, or did not come back. Each flowerbed has a name tag.\n\n"
         "After the fire Pell cut off his own face. Whether from guilt or because the children's faces haunted him, nobody knows. He is the only monster in the house who is not a child: he is the human cruelty that made the rest possible.\n\n"
         "Pell moves only when you are not looking, and he moves fast. He cannot be reasoned with.",
         "AVOID: break line of sight, hide behind walls.\nOUTSMART: keep your lantern lit and on him. While you see him, he freezes.\nMERCY: none. He is the only ghost who cannot be forgiven.",
         "Rustling, breathing and the clipped snip of shears. He is heard before he is seen."},
        {"THE CHOIR", "Three children, one chord", "The Chorus Ward: joined throats.",
         "In the cellar ward three children were selected for the programme's most ambitious experiment. Crane joined their vocal cords so that they could produce only one note between them, a calm, steady chord. He called it harmony. They called it the Chorus.\n\n"
         "The surgery left them blind and fused at the throat. They stand together in the boiler cellar, singing their single chord, which cracks the mortar when they are afraid.\n\n"
         "They cannot see. They find you by sound: footsteps, breath, a dropped bottle. They are not malicious. They have been afraid for twenty-five years.",
         "AVOID: walk slowly. Running is loud.\nOUTSMART: throw a bottle to lure them away.\nMERCY: stand still, hold your breath (SHIFT+C) for a few seconds, and let them sing in peace.",
         "A whispering chorus and a held, dissonant chord. When calm, the voices blend into one."},
        {"ELI", "The Doll", "Nora's little brother, five years old.",
         "Eli was Nora's brother. He suffered night terrors and Dr. Crane gave him a porcelain mask to wear \"to calm him,\" which also kept his eyes shut. On the last night he was strapped into the cellar chair, crying silently.\n\n"
         "He called Nora's name through the ward door. She locked it. Since then Eli has been playing the last game he remembers, hide-and-seek. He counts, he hides, he giggles, he appears behind you. He is not hunting you; he wants you to find him.\n\n"
         "He still carries his blanket. The fourth sigil is the blanket.",
         "AVOID: keep your lantern on. He is most dangerous in the dark.\nOUTSMART: shine the lantern on him to stun him briefly.\nMERCY: close your eyes with CTRL while he is near, then open them. Peek-a-boo. Do not hurt him: harming him changes the ending.",
         "A toy-piano plink and a child's breathy laughter, over the Hush, Little Light melody played out of tune."},
        {"DR. CRANE", "The Warden", "Silas Crane, fused with the burnt ward door.",
         "Dr. Silas Crane built the Quiet Programme. He died in the fire when Nora locked the door; his body fused with the charred ward door and the chains used to restrain children. What walks the house now is a burnt iron shape with a ring of keys.\n\n"
         "The Warden is both monster and jailer. He fears what Nora's confession would do: if the children forgive her, the house ends and so does he. So he stops her. He wakes once she has collected three sigils, and from then on he hunts her through the whole house.\n\n"
         "He walks through doors. He hears sprinting from far away. He loses interest only when you are inside candlelight.",
         "AVOID: stay out of sight and walk. He hears running.\nOUTSMART: hold the focused lantern on him to stun him for two seconds, then run.\nMERCY: none in the usual sense. He is only defeated by confession.",
         "Iron chains dragging over stone, a ring of keys, heavy breathing and a low rumble."},
    };
    m_selected = 0;
}

void StoryBrowser::init(const QRect& b) {
    m_items.clear();
    int x = 36, y = 84, w = 310, h = 32, gap = 4;
    for (const Page& pg : m_pages) {
        m_items.push_back({pg.title, QRect(x, y, w, h)});
        y += h + gap;
    }
    m_items.push_back({"WATCH THE INTRO MOVIE", QRect(x, y + 8, w, h)});
    m_items.push_back({"BACK TO MENU", QRect(x, y + 8 + h + gap, w, h)});
    m_content = QRect(372, 84, b.width() - 372 - 36, b.height() - 130);
}

void StoryBrowser::reset() { select(0); }

void StoryBrowser::select(int i) {
    m_selected = i;
    m_typed = 0.0f;
    m_scroll = 0.0f;
}

void StoryBrowser::update(float dt) { m_typed += dt * 420.0f; }

QRect StoryBrowser::watchButton() const { return QRect(m_content.right() - 330, m_content.top() + 20, 306, 40); }

QRect StoryBrowser::ghostCardRect(int i) const {
    int w = (m_content.width() - 48 - 4 * 12) / 5;
    return QRect(m_content.left() + 24 + i * (w + 12), m_content.top() + 100, w, 190);
}

void StoryBrowser::onWheel(int delta) {
    m_scroll = std::clamp(m_scroll - delta * 0.5f, 0.0f, m_maxScroll);
}

void StoryBrowser::onMouseMove(const QPoint& pos) {
    m_hover = -1;
    for (size_t i = 0; i < m_items.size(); ++i)
        if (m_items[i].rect.contains(pos)) m_hover = static_cast<int>(i);
    m_hoverWatch = m_selected < static_cast<int>(m_pages.size()) && m_pages[m_selected].ending >= 0 && watchButton().contains(pos);
    m_ghostHover = -1;
    if (m_selected < static_cast<int>(m_pages.size()) && m_pages[m_selected].ghosts)
        for (int i = 0; i < 5; ++i) if (ghostCardRect(i).contains(pos)) m_ghostHover = i;
}

StoryAction StoryBrowser::onClick(const QPoint& pos) {
    int n = static_cast<int>(m_pages.size());
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (!m_items[i].rect.contains(pos)) continue;
        int idx = static_cast<int>(i);
        if (idx < n) { select(idx); return StoryAction::None; }
        return idx == n ? StoryAction::WatchIntro : StoryAction::Back;
    }
    if (m_selected < n && m_pages[m_selected].ending >= 0 && watchButton().contains(pos)) {
        m_endingToWatch = m_pages[m_selected].ending;
        return StoryAction::WatchEnding;
    }
    if (m_selected < n && m_pages[m_selected].ghosts) {
        for (int i = 0; i < 5; ++i)
            if (ghostCardRect(i).contains(pos)) { m_ghostSel = i; m_typed = 0.0f; m_scroll = 0.0f; return StoryAction::None; }
    }
    if (m_content.contains(pos)) m_typed = 100000.0f;   // finish the typewriter at once
    return StoryAction::None;
}

// Little animated portraits so each ghost is recognisable on sight.
static void drawPortrait(QPainter& p, const QRect& r, int idx, float t, bool selected) {
    p.save();
    p.setClipRect(r);
    QLinearGradient bg(r.topLeft(), r.bottomLeft());
    static const QColor top[5] = {QColor(40, 58, 104), QColor(26, 70, 58), QColor(52, 52, 78), QColor(100, 66, 58), QColor(110, 34, 22)};
    static const QColor bot[5] = {QColor(10, 14, 28), QColor(6, 20, 16), QColor(12, 12, 20), QColor(30, 18, 16), QColor(26, 8, 6)};
    bg.setColorAt(0, top[idx]);
    bg.setColorAt(1, bot[idx]);
    p.fillRect(r, bg);
    float s = r.height() / 190.0f;
    static const QColor aura[5] = {QColor(150, 190, 255), QColor(150, 230, 190), QColor(255, 200, 140), QColor(255, 200, 170), QColor(255, 130, 70)};
    glow(p, QPointF(r.center().x(), r.center().y() - 10 * s), r.width() * 0.9f, aura[idx], 0.42f + 0.1f * std::sin(t * 2.0f + idx));   // ghostly halo behind the figure
    p.translate(r.center().x(), r.bottom() - 6 * s);
    p.scale(s * 0.42f, s * 0.42f);
    float sway = std::sin(t * 1.4f + idx) * 3.0f;
    QColor sil(8, 9, 14);
    p.setPen(Qt::NoPen);
    switch (idx) {
    case 0:   // Ada: child with a red ribbon and silent tears
        child(p, QPointF(0, 0), 2.3f, sil, QColor(200, 30, 45), sway);
        for (int i = 0; i < 4; ++i) {
            float life = std::fmod(t * 0.7f + i * 0.25f, 1.0f);
            glow(p, QPointF(-4 + (i % 2) * 14, -285 + life * 120), 14, QColor(170, 200, 255), (1 - life) * 0.9f);
        }
        break;
    case 1: {   // Pell: tall figure, blank face, shears
        p.setBrush(sil);
        p.drawRoundedRect(QRectF(-34, -330, 68, 330), 22, 22);
        p.setBrush(QColor(210, 205, 195));
        p.drawEllipse(QPointF(sway, -360), 26, 32);               // blank, hollow face
        p.setPen(QPen(QColor(200, 210, 210, 220), 5));
        float sn = std::fabs(std::sin(t * 5.0f)) * 14.0f;
        p.drawLine(QPointF(40, -200), QPointF(88, -250 - sn));
        p.drawLine(QPointF(40, -200), QPointF(88, -250 + sn));
        break;
    }
    case 2:   // Choir: three children joined at the throat
        for (int i = 0; i < 3; ++i) child(p, QPointF(-120 + i * 120, 0), 1.9f, sil, QColor(120, 120, 140), std::sin(t * 1.3f + i) * 3.0f);
        {
            float pulse = 0.5f + 0.5f * std::sin(t * 4.0f);
            p.setPen(QPen(QColor(255, 205, 130, static_cast<int>(210 * pulse)), 8, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(-120, -238), QPointF(120, -238));
            glow(p, QPointF(0, -238), 130, QColor(255, 190, 110), 0.5f * pulse);
        }
        break;
    case 3: {   // Eli: small child, porcelain mask, blanket
        child(p, QPointF(0, 0), 2.0f, QColor(20, 18, 24), QColor(255, 220, 140), sway);
        p.setBrush(QColor(236, 232, 222));
        p.drawEllipse(QPointF(sway * 0.4f, -270), 36, 40);       // porcelain mask
        p.setPen(QPen(QColor(30, 30, 30), 4));
        p.drawLine(QPointF(-18, -282), QPointF(-4, -266)); p.drawLine(QPointF(-4, -282), QPointF(-18, -266));   // stitched eyes
        p.drawLine(QPointF(8, -282), QPointF(22, -266)); p.drawLine(QPointF(22, -282), QPointF(8, -266));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(210, 140, 170));
        p.drawRoundedRect(QRectF(-80, -150, 50, 70), 8, 8);       // blanket
        break;
    }
    default: {  // Warden: burnt door shape with chains and keys
        flames(p, t, QRectF(-150, -70, 300, 90), 0.9f, 3);
        p.setBrush(sil);
        p.drawRect(QRectF(-80, -430, 160, 430));
        p.setBrush(QColor(28, 12, 8));
        p.drawRect(QRectF(-58, -400, 116, 170));
        p.drawRect(QRectF(-58, -210, 116, 170));
        p.setBrush(QColor(8, 8, 10));
        p.drawEllipse(QPointF(0, -455), 30, 36);                  // iron-masked head
        p.setPen(QPen(QColor(120, 120, 130), 5));
        for (int i = 0; i < 3; ++i) p.drawLine(QPointF(-80, -300 + i * 50), QPointF(-130 + std::sin(t * 2 + i) * 6, -240 + i * 60));
        glow(p, QPointF(86, -170), 70, QColor(255, 180, 70), 0.5f);
        break;
    }
    }
    p.restore();
    p.save();
    p.setPen(QPen(selected ? QColor(255, 225, 150) : QColor(90, 86, 96), selected ? 2.5 : 1.2));
    p.setBrush(Qt::NoBrush);
    p.drawRect(r);
    p.restore();
}

void StoryBrowser::drawBody(QPainter& p, const QString& text, const QRect& area) {
    QString shown = text.left(static_cast<int>(m_typed));
    p.save();
    p.setClipRect(area);
    p.setPen(QColor(222, 216, 206));
    p.setFont(cachedFont("Georgia", 18));
    QFontMetrics fm(cachedFont("Georgia", 18));
    QRect full = fm.boundingRect(QRect(0, 0, area.width() - 14, 100000), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, text);
    m_maxScroll = std::max(0, full.height() - area.height() + 10);
    m_scroll = std::clamp(m_scroll, 0.0f, m_maxScroll);
    p.drawText(QRect(area.left(), area.top() - static_cast<int>(m_scroll), area.width() - 14, full.height() + 40), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, shown);
    p.restore();
    if (m_maxScroll > 0.0f) {   // scrollbar
        float frac = area.height() / static_cast<float>(full.height());
        int barH = std::max(30, static_cast<int>(area.height() * frac));
        int barY = area.top() + static_cast<int>((area.height() - barH) * (m_scroll / m_maxScroll));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(60, 58, 64));
        p.drawRoundedRect(QRect(area.right() - 8, area.top(), 6, area.height()), 3, 3);
        p.setBrush(QColor(235, 205, 140));
        p.drawRoundedRect(QRect(area.right() - 8, barY, 6, barH), 3, 3);
    }
}

void StoryBrowser::drawGhostGallery(QPainter& p, float time) {
    for (int i = 0; i < 5; ++i) {
        QRect r = ghostCardRect(i);
        drawPortrait(p, r, i, time, i == m_ghostSel || i == m_ghostHover);
        p.setPen(i == m_ghostSel ? QColor(255, 235, 175) : QColor(205, 198, 190));
        p.setFont(cachedFont("Georgia", 15, true, false, 1.5));
        p.drawText(QRect(r.left(), r.bottom() + 4, r.width(), 22), Qt::AlignCenter, m_ghosts[i].name);
    }
    const GhostCard& g = m_ghosts[m_ghostSel];
    p.setPen(QColor(255, 240, 215));
    p.setFont(cachedFont("Georgia", 26, true));
    p.drawText(QRect(m_content.left() + 24, m_content.top() + 326, m_content.width() - 48, 34), Qt::AlignLeft | Qt::AlignVCenter, g.name + "  -  " + g.alias);
    p.setPen(QColor(210, 150, 130));
    p.setFont(cachedFont("Georgia", 16, false, true));
    p.drawText(QRect(m_content.left() + 24, m_content.top() + 360, m_content.width() - 48, 24), Qt::AlignLeft | Qt::AlignVCenter, g.subtitle);
    QString text = g.backstory + "\n\nHOW TO DEAL WITH THEM\n" + g.counter + "\n\nTHEIR SOUND\n" + g.sound;
    drawBody(p, text, QRect(m_content.left() + 24, m_content.top() + 392, m_content.width() - 48, m_content.height() - 408));
}

void StoryBrowser::render(QPainter& p, const QRect& b, float time) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(b, QColor(5, 5, 8, 236));
    p.setPen(QColor(235, 205, 140));
    p.setFont(cachedFont("Georgia", 30, true, false, 6.0));
    p.drawText(QRect(36, 18, 800, 56), Qt::AlignLeft | Qt::AlignVCenter, "STORY & ENDINGS");

    for (size_t i = 0; i < m_items.size(); ++i) {
        const Item& it = m_items[i];
        bool sel = static_cast<int>(i) == m_selected;
        bool hov = static_cast<int>(i) == m_hover;
        p.setPen(QPen(sel ? QColor(235, 205, 140, 230) : QColor(70, 66, 74, 200), sel ? 1.8 : 1.0));
        p.setBrush(sel ? QColor(52, 44, 34, 220) : (hov ? QColor(36, 36, 48, 220) : QColor(16, 16, 22, 200)));
        p.drawRoundedRect(it.rect, 5, 5);
        p.setPen(sel ? QColor(255, 235, 175) : (hov ? QColor(240, 230, 210) : QColor(200, 196, 190)));
        p.setFont(cachedFont("Courier New", 13, true, false, 1.0));
        p.drawText(it.rect, Qt::AlignCenter, it.label);
    }

    p.setPen(QPen(QColor(235, 205, 140, 80), 1.2));
    p.setBrush(QColor(10, 10, 14, 215));
    p.drawRoundedRect(m_content, 8, 8);
    if (m_selected < static_cast<int>(m_pages.size())) {
        const Page& pg = m_pages[m_selected];
        p.setPen(QColor(255, 240, 215));
        p.setFont(cachedFont("Georgia", 30, true));
        p.drawText(QRect(m_content.left() + 24, m_content.top() + 10, m_content.width() - 380, 44), Qt::AlignLeft | Qt::AlignVCenter, pg.title);
        p.setPen(QColor(210, 150, 130));
        p.setFont(cachedFont("Georgia", 18, false, true));
        p.drawText(QRect(m_content.left() + 24, m_content.top() + 54, m_content.width() - 48, 28), Qt::AlignLeft | Qt::AlignVCenter, pg.subtitle);
        p.setPen(QPen(QColor(190, 40, 40, 160), 1.5));
        p.drawLine(m_content.left() + 24, m_content.top() + 88, m_content.right() - 24, m_content.top() + 88);

        if (pg.ghosts) {
            drawGhostGallery(p, time);
        } else {
            drawBody(p, pg.body, QRect(m_content.left() + 24, m_content.top() + 100, m_content.width() - 48, m_content.height() - 116));
        }
        if (pg.ending >= 0) {
            QRect wb = watchButton();
            p.setPen(QPen(QColor(235, 205, 140, m_hoverWatch ? 255 : 170), 1.6));
            p.setBrush(m_hoverWatch ? QColor(70, 56, 36, 230) : QColor(30, 26, 20, 220));
            p.drawRoundedRect(wb, 6, 6);
            p.setPen(QColor(255, 235, 175));
            p.setFont(cachedFont("Courier New", 15, true, false, 1.0));
            p.drawText(wb, Qt::AlignCenter, QString::fromUtf8("\xE2\x96\xB6  WATCH THIS ENDING"));
        }
    }
    p.setPen(QColor(150, 146, 140));
    p.setFont(cachedFont("Georgia", 14, false, true));
    p.drawText(QRect(372, b.height() - 40, b.width() - 420, 24), Qt::AlignLeft, "Pick a page on the left. Mouse wheel scrolls. Click the text to show it all at once.");
    p.restore();
}

} // namespace hm
