#include "JournalWidget.h"
#include <QFont>
#include <algorithm>

namespace hm {

JournalOverlay::JournalOverlay() = default;

void JournalOverlay::init(const QRect&) {
    m_selectedWing = -1;
    m_selectedNoteIndex = 0;
    m_hintHoldTimer = 0.0f;
    m_hintTriggered = false;
    m_activeHint.clear();
}

void JournalOverlay::update(float dt, bool hintHeld) {
    if (hintHeld) {
        m_hintHoldTimer += dt;
        if (m_hintHoldTimer >= m_hintHoldRequired) {
            m_hintTriggered = true;
        }
    } else {
        m_hintHoldTimer = std::max(0.0f, m_hintHoldTimer - dt * 2.0f);
    }
}

void JournalOverlay::nextNote() {
    m_selectedNoteIndex++;
}

void JournalOverlay::prevNote() {
    if (m_selectedNoteIndex > 0) m_selectedNoteIndex--;
}

void JournalOverlay::selectWing(int wing) {
    m_selectedWing = wing;
    m_selectedNoteIndex = 0;
}

void JournalOverlay::updateHint(int currentWing) {
    switch (currentWing) {
    case 0:
        m_activeHint = "OBJECTIVE: Inspect the grand staircase in the Foyer. Find the Brass Key and lantern oil.";
        break;
    case 1:
        m_activeHint = "OBJECTIVE: Search the Library study. The cipher symbols are etched into book spines.";
        break;
    case 2:
        m_activeHint = "OBJECTIVE: Rotate the glass mirrors in the Conservatory to direct the light beam into the crystal.";
        break;
    case 3:
        m_activeHint = "OBJECTIVE: Balance the 4 steam valves in the Boiler Cellar to silence the pressure whistle.";
        break;
    case 4:
        m_activeHint = "OBJECTIVE: Play the 3-note melody on the Nursery music box. Listen carefully for Eli's hum.";
        break;
    case 5:
        m_activeHint = "OBJECTIVE: Place all five Sigils upon the Chapel altar. Face Dr. Crane.";
        break;
    default:
        m_activeHint = "OBJECTIVE: Explore Hollowmere Manor. Find the 5 sigils to unlock the Chapel.";
        break;
    }
}

void JournalOverlay::render(QPainter& p, const QRect& bounds, const Journal& journal, int currentWing) {
    updateHint(currentWing);

    // Full screen parchment vignette
    p.fillRect(bounds, QColor(10, 12, 16, 230));

    int padX = 80;
    int padY = 50;
    QRect paperRect(padX, padY, bounds.width() - padX * 2, bounds.height() - padY * 2);

    // Parchment paper background
    p.setPen(QPen(QColor(140, 110, 70), 2.0f));
    p.setBrush(QColor(235, 225, 205));
    p.drawRoundedRect(paperRect, 6, 6);

    // Header
    QFont titleFont("Georgia", 22, QFont::Bold);
    p.setFont(titleFont);
    p.setPen(QColor(40, 28, 20));
    p.drawText(paperRect.left() + 30, paperRect.top() + 45, "NORA'S FIELD JOURNAL");

    // Subtitle & Wing filters
    QFont filterFont("Courier New", 10, QFont::Bold);
    p.setFont(filterFont);
    p.setPen(QColor(110, 90, 70));
    QString wingStr = (m_selectedWing == -1) ? "ALL WINGS" : QString("WING %1").arg(m_selectedWing);
    p.drawText(paperRect.left() + 30, paperRect.top() + 75, QString("[FILTERS: %1] (1-5 to filter, 0 for All, UP/DOWN to browse)").arg(wingStr));

    // Stats: Collected Notes
    QFont statFont("Courier New", 10, QFont::Normal);
    p.setFont(statFont);
    p.setPen(QColor(120, 40, 40));
    p.drawText(paperRect.right() - 320, paperRect.top() + 45, QString("NOTES COLLECTED: %1 / 40").arg(journal.allCollectedNotes().size()));
    p.drawText(paperRect.right() - 320, paperRect.top() + 70, QString("WING OBJECTIVE: WING %1").arg(currentWing));

    // Filter notes
    std::vector<NoteData> filteredNotes;
    for (const auto& note : journal.allCollectedNotes()) {
        if (m_selectedWing == -1 || static_cast<int>(note.wing) == m_selectedWing) {
            filteredNotes.push_back(note);
        }
    }

    // Two-column layout: Left = Notes list (30%), Right = Note content (70%)
    int colSplit = paperRect.left() + static_cast<int>(paperRect.width() * 0.32f);

    // Vertical separator
    p.setPen(QPen(QColor(180, 160, 130), 1.0f));
    p.drawLine(colSplit, paperRect.top() + 95, colSplit, paperRect.bottom() - 60);

    // Left Column: Note titles
    if (m_selectedNoteIndex >= static_cast<int>(filteredNotes.size())) {
        m_selectedNoteIndex = std::max(0, static_cast<int>(filteredNotes.size()) - 1);
    }

    int listY = paperRect.top() + 120;
    QFont listFont("Georgia", 11, QFont::Normal);
    p.setFont(listFont);

    if (filteredNotes.empty()) {
        p.setPen(QColor(120, 110, 95));
        p.drawText(paperRect.left() + 30, listY, "No journal notes found in this wing.");
    } else {
        for (size_t i = 0; i < filteredNotes.size(); ++i) {
            if (listY > paperRect.bottom() - 80) break;
            if (static_cast<int>(i) == m_selectedNoteIndex) {
                p.fillRect(paperRect.left() + 20, listY - 18, colSplit - paperRect.left() - 30, 26, QColor(215, 200, 175));
                p.setPen(QColor(140, 30, 30));
            } else {
                p.setPen(QColor(50, 40, 30));
            }
            p.drawText(paperRect.left() + 25, listY, QString::fromStdString(filteredNotes[i].title));
            listY += 30;
        }
    }

    // Right Column: Active Note Text
    if (!filteredNotes.empty() && m_selectedNoteIndex < static_cast<int>(filteredNotes.size())) {
        const auto& activeNote = filteredNotes[m_selectedNoteIndex];

        QFont noteTitleFont("Georgia", 16, QFont::Bold);
        p.setFont(noteTitleFont);
        p.setPen(QColor(30, 20, 15));
        p.drawText(colSplit + 30, paperRect.top() + 130, QString::fromStdString(activeNote.title));

        QFont noteAuthorFont("Georgia", 11, QFont::StyleItalic);
        p.setFont(noteAuthorFont);
        p.setPen(QColor(110, 90, 70));
        p.drawText(colSplit + 30, paperRect.top() + 155, QString("Wing %1 Entry").arg(static_cast<int>(activeNote.wing)));

        // Note Body with word wrap
        QFont noteBodyFont("Georgia", 13, QFont::Normal);
        p.setFont(noteBodyFont);
        p.setPen(QColor(40, 32, 25));
        QRect bodyRect(colSplit + 30, paperRect.top() + 185, paperRect.right() - colSplit - 60, paperRect.height() - 260);
        p.drawText(bodyRect, Qt::TextWordWrap, QString::fromStdString(activeNote.text));
    }

    // Bottom Bar: Hint System (Hold H for 5s)
    p.fillRect(paperRect.left(), paperRect.bottom() - 55, paperRect.width(), 55, QColor(220, 210, 190));
    p.setPen(QPen(QColor(170, 150, 120), 1.0f));
    p.drawLine(paperRect.left(), paperRect.bottom() - 55, paperRect.right(), paperRect.bottom() - 55);

    if (m_hintTriggered) {
        QFont hintF("Courier New", 10, QFont::Bold);
        p.setFont(hintF);
        p.setPen(QColor(160, 25, 25));
        p.drawText(paperRect.left() + 30, paperRect.bottom() - 22, m_activeHint);
    } else {
        QFont hintPromptF("Courier New", 9, QFont::Normal);
        p.setFont(hintPromptF);
        p.setPen(QColor(90, 80, 70));
        QString hintStr = QString("HOLD [H] FOR OBJECTIVE HINT (%1%)").arg(static_cast<int>((m_hintHoldTimer / m_hintHoldRequired) * 100.0f));
        p.drawText(paperRect.left() + 30, paperRect.bottom() - 22, hintStr);

        // Progress bar for hint
        if (m_hintHoldTimer > 0.0f) {
            int hbarW = 140;
            int hbarH = 6;
            int hx = paperRect.left() + 350;
            int hy = paperRect.bottom() - 32;
            p.fillRect(hx, hy, hbarW, hbarH, QColor(180, 170, 150));
            p.fillRect(hx, hy, static_cast<int>(hbarW * (m_hintHoldTimer / m_hintHoldRequired)), hbarH, QColor(180, 40, 40));
        }
    }
}

} // namespace hm
