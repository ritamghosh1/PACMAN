#pragma once

#include <QRect>
#include <QPainter>
#include <vector>
#include <QString>
#include "story/Journal.h"
#include "story/Notes.h"
#include "story/Fragment.h"

namespace hm {

class JournalOverlay {
public:
    JournalOverlay();

    void init(const QRect& bounds);
    void update(float dt, bool hintHeld);
    void render(QPainter& p, const QRect& bounds, const Journal& journal, int currentWing);

    void nextNote();
    void prevNote();
    void selectWing(int wing); // -1 for all, 0-5 for wings

    bool isHintTriggered() const { return m_hintTriggered; }
    QString currentHint() const { return m_activeHint; }

private:
    int m_selectedWing = -1;
    int m_selectedNoteIndex = 0;
    float m_hintHoldTimer = 0.0f;
    const float m_hintHoldRequired = 5.0f;
    bool m_hintTriggered = false;
    QString m_activeHint;

    void updateHint(int currentWing);
};

} // namespace hm
