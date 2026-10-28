#include "StoryBrowser.h"
#include "FontCache.h"
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

namespace hm {

StoryBrowser::StoryBrowser() {
    m_pages = {
        {"THE STORY", "A psychological horror in a haunted orphanage",
         "Hollowmere is St. Aldous Home for Children, an orphanage that burned in 1998. Nora Hale was a patient there at nine years old, together with her little brother Eli. The director, Dr. Silas Crane, ran \"The Quiet Programme\": children who cried were silenced.\n\n"
         "On the night of the fire Nora ran for the exit and locked the ward door behind her to stop Crane. Eli and the other children were still inside.\n\n"
         "Twenty-five years later Nora's daughter Mae vanishes. A music box arrives playing Nora's forgotten lullaby, with a black-sealed letter addressed to Hollowmere. The dead children have taken Mae, and they want to be forgiven.\n\n"
         "The manor has four wings. Each keeps a Memory Sigil, a piece of that night, guarded by the ghost of someone who was hurt: Ada the Weeper (silenced for crying), Mr. Pell the Hollow Man (the orderly who \"pruned\" children), the blind Choir (three children with joined throats) and Eli the Doll (who only wants a game of hide-and-seek). Dr. Crane, fused to the burnt ward door, hunts Nora as the Warden.\n\n"
         "Nora carries the four sigils to the chapel altar. What she did to the ghosts along the way decides how the night ends."},
        {"HOW IT PLAYS", "First-person horror maze, no voice acting",
         "GOAL: find four memory sigils (one per wing), use the key found in each wing to open the next, then place the sigils on the chapel altar.\n\n"
         "EACH GHOST HAS THREE ANSWERS: avoid it, outsmart it, or show it mercy.\n"
         "  Ada the Weeper: creep (C) past her, burn her with the focused lantern (right mouse), or hold H and click on the beat to hum her lullaby.\n"
         "  Mr. Pell: he only moves when unseen. Keep your lantern on him.\n"
         "  The Choir: blind. Walk, never run; hold your breath (SHIFT+C).\n"
         "  Eli: close your eyes (CTRL) while he is near, then open them: peek-a-boo.\n"
         "  The Warden: runs once you hold three sigils. Sprint (SHIFT) or stun him with the focused lantern.\n\n"
         "Candle-lit rooms (the foyer and each wing entrance) are safe.\n\n"
         "STORY IS TOLD WITHOUT VOICES: typewriter text you click through, animated flashbacks after every sigil, and sound. Each character has a signature tick and motif so you recognise who is speaking by ear.\n\n"
         "TECH: Qt 5 + OpenGL 3.3 core. All geometry, textures and audio are generated in code."},
        {"ENDING I", "THE DAWN AT HOLLOWMERE  (best ending)",
         "HOW TO GET IT: spare all three kindred ghosts (hum to Ada, play peek-a-boo with Eli, let the Choir sing) and read at least 8 of the 20 notes.\n\n"
         "THE ENDING: The sigils settle into the altar and the ward door swings open. Nora says it aloud at last: she locked them in, and she is sorry. The children forgive her. The house burns away into morning, and when she wakes, Mae's red ribbon is warm in her hand.", 0},
        {"ENDING II", "THE NEW CARETAKER  (dark ending)",
         "HOW TO GET IT: drive off or stun three or more ghosts with the lantern instead of showing mercy.\n\n"
         "THE ENDING: Ash drifts through the foyer like black snow. The cold seeps into Nora until her breath no longer steams. She lifts the iron keys from the floor. Someone must mind the wards.", 1},
        {"ENDING III", "THE ENDLESS DRIVE  (default ending)",
         "HOW TO GET IT: reach the altar without sparing everyone and without banishing three ghosts.\n\n"
         "THE ENDING: Nora drags Mae to the car and drives without looking back. The radio crackles into a music box, a lullaby in three-four time. In the mirror, the manor waits. The road remembers her.", 2},
        {"ENDING IV", "CHORUS OF THE INNOCENT  (bittersweet)",
         "HOW TO GET IT: spare all three ghosts but read fewer than 8 notes.\n\n"
         "THE ENDING: Nora kneels at the altar and sings the children to rest. Their small shapes dissolve into candlelight, and Mae walks out alone into the dawn. The house keeps Nora as its quiet guardian.", 3},
        {"ENDING V", "THE DOCTOR'S ARCHIVE  (secret ending)",
         "HOW TO GET IT: find and read all 16 wing notes plus Nora's resignation letter in the chapel.\n\n"
         "THE ENDING: The hidden drawer clicks open. Inside lies a yellowed medical chart in Dr. Crane's hand: Patient 404, Nora Hale. The truth washes over the shadows like daylight over broken glass.", 4},
    };
    m_selected = 0;
}

void StoryBrowser::init(const QRect& b) {
    m_items.clear();
    int x = 60, y = 120, w = 300, h = 46, gap = 10;
    for (const Page& pg : m_pages) {
        m_items.push_back({pg.title, QRect(x, y, w, h)});
        y += h + gap;
    }
    m_items.push_back({"WATCH THE INTRO MOVIE", QRect(x, y + 14, w, h)});
    m_items.push_back({"BACK TO MENU", QRect(x, y + 14 + h + gap, w, h)});
    m_content = QRect(400, 90, b.width() - 400 - 60, b.height() - 180);
}

void StoryBrowser::reset() { select(0); }

void StoryBrowser::select(int i) {
    m_selected = i;
    m_typed = 0.0f;
}

void StoryBrowser::update(float dt) { m_typed += dt * 260.0f; }   // fast typewriter: a screenful in a couple of seconds

QRect StoryBrowser::watchButton() const { return QRect(m_content.left() + 24, m_content.bottom() - 64, 300, 44); }

void StoryBrowser::onMouseMove(const QPoint& pos) {
    m_hover = -1;
    for (size_t i = 0; i < m_items.size(); ++i)
        if (m_items[i].rect.contains(pos)) m_hover = static_cast<int>(i);
    m_hoverWatch = m_selected < static_cast<int>(m_pages.size()) && m_pages[m_selected].ending >= 0 && watchButton().contains(pos);
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
    // Clicking the text finishes the typewriter at once
    if (m_content.contains(pos)) m_typed = 100000.0f;
    return StoryAction::None;
}

void StoryBrowser::render(QPainter& p, const QRect& b, float time) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(b, QColor(5, 5, 8, 232));
    p.setPen(QColor(235, 205, 140));
    p.setFont(cachedFont("Georgia", 34, true, false, 6.0));
    p.drawText(QRect(60, 30, 800, 56), Qt::AlignLeft | Qt::AlignVCenter, "STORY & ENDINGS");

    for (size_t i = 0; i < m_items.size(); ++i) {
        const Item& it = m_items[i];
        bool sel = static_cast<int>(i) == m_selected;
        bool hov = static_cast<int>(i) == m_hover;
        p.setPen(QPen(sel ? QColor(235, 205, 140, 230) : QColor(70, 66, 74, 200), sel ? 1.8 : 1.0));
        p.setBrush(sel ? QColor(52, 44, 34, 220) : (hov ? QColor(36, 36, 48, 220) : QColor(16, 16, 22, 200)));
        p.drawRoundedRect(it.rect, 5, 5);
        p.setPen(sel ? QColor(255, 235, 175) : (hov ? QColor(240, 230, 210) : QColor(200, 196, 190)));
        p.setFont(cachedFont("Courier New", 15, true, false, 1.5));
        p.drawText(it.rect, Qt::AlignCenter, it.label);
    }

    p.setPen(QPen(QColor(235, 205, 140, 80), 1.2));
    p.setBrush(QColor(10, 10, 14, 215));
    p.drawRoundedRect(m_content, 8, 8);
    if (m_selected < static_cast<int>(m_pages.size())) {
        const Page& pg = m_pages[m_selected];
        p.setPen(QColor(255, 240, 215));
        p.setFont(cachedFont("Georgia", 32, true));
        p.drawText(QRect(m_content.left() + 24, m_content.top() + 14, m_content.width() - 48, 46), Qt::AlignLeft | Qt::AlignVCenter, pg.title);
        p.setPen(QColor(210, 150, 130));
        p.setFont(cachedFont("Georgia", 19, false, true));
        p.drawText(QRect(m_content.left() + 24, m_content.top() + 58, m_content.width() - 48, 30), Qt::AlignLeft | Qt::AlignVCenter, pg.subtitle);
        p.setPen(QPen(QColor(190, 40, 40, 160), 1.5));
        p.drawLine(m_content.left() + 24, m_content.top() + 96, m_content.right() - 24, m_content.top() + 96);

        // Typewriter reveal of the body
        QString shown = pg.body.left(static_cast<int>(m_typed));
        p.setPen(QColor(222, 216, 206));
        p.setFont(cachedFont("Georgia", 19));
        QRect body(m_content.left() + 24, m_content.top() + 108, m_content.width() - 48, m_content.height() - (pg.ending >= 0 ? 190 : 130));
        p.drawText(body, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, shown);
        if (m_typed < pg.body.size() && (static_cast<int>(time * 3) & 1)) {
            p.setPen(QColor(235, 205, 140));
            p.drawText(QRect(m_content.right() - 40, m_content.bottom() - 36, 20, 24), Qt::AlignCenter, "|");
        }
        if (pg.ending >= 0) {
            QRect wb = watchButton();
            p.setPen(QPen(QColor(235, 205, 140, m_hoverWatch ? 255 : 170), 1.6));
            p.setBrush(m_hoverWatch ? QColor(70, 56, 36, 230) : QColor(30, 26, 20, 220));
            p.drawRoundedRect(wb, 6, 6);
            p.setPen(QColor(255, 235, 175));
            p.setFont(cachedFont("Courier New", 16, true, false, 1.5));
            p.drawText(wb, Qt::AlignCenter, QString::fromUtf8("\xE2\x96\xB6  WATCH THIS ENDING"));
        }
    }
    p.setPen(QColor(150, 146, 140));
    p.setFont(cachedFont("Georgia", 14, false, true));
    p.drawText(QRect(400, b.height() - 70, b.width() - 460, 24), Qt::AlignLeft, "Click an entry on the left. Click the text to show it all at once.");
    p.restore();
}

} // namespace hm
