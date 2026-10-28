#include "Dialogue.h"
#include "audio/AudioEngine.h"
#include "ui/FontCache.h"
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainterPath>
#include <QStringList>
#include <algorithm>
#include <cmath>

namespace hm {

namespace {
struct Style { const char* name; QColor color; const char* family; bool italic; };

const Style& styleFor(Speaker s) {
    static const Style table[] = {
        {"",          QColor(205, 200, 190), "Georgia",     true},   // Narrator
        {"NORA",      QColor(232, 205, 170), "Georgia",     false},
        {"MAE",       QColor(245, 170, 195), "Georgia",     false},
        {"DR. CRANE", QColor(150, 185, 225), "Courier New", false},
        {"ADA",       QColor(210, 120, 120), "Georgia",     true},
        {"ELI",       QColor(235, 215, 140), "Georgia",     false},
        {"HOLLOWMERE",QColor(170, 170, 180), "Georgia",     true},   // Manor
    };
    return table[std::clamp(static_cast<int>(s), 0, static_cast<int>(Speaker::Count) - 1)];
}

bool isPausePunct(QChar c) {
    return c == '.' || c == ',' || c == '!' || c == '?' || c == ';' || c == ':' || c == QChar(0x2026) || c == QChar(0x2014);
}
float pauseFor(QChar c) {
    if (c == '.' || c == '!' || c == '?' || c == QChar(0x2026)) return 0.32f;
    if (c == QChar(0x2014)) return 0.22f;
    return 0.14f;
}
} // namespace

QString DialogueBox::displayName(Speaker s) { return QString::fromUtf8(styleFor(s).name); }
QColor DialogueBox::accent(Speaker s) { return styleFor(s).color; }

void DialogueBox::push(const DialogueLine& line) {
    m_queue.push_back(line);
    if (!m_hasCurrent) startNext();
}

void DialogueBox::push(const std::vector<DialogueLine>& lines) {
    for (const auto& l : lines) m_queue.push_back(l);
    if (!m_hasCurrent && !m_queue.empty()) startNext();
}

void DialogueBox::clear() {
    m_queue.clear();
    m_hasCurrent = false;
    m_hasLast = false;
    m_shown = 0.0f;
    m_lineCounter = 0;
}

void DialogueBox::startNext() {
    if (m_queue.empty()) {
        m_hasCurrent = false;
        return;
    }
    m_current = m_queue.front();
    m_queue.pop_front();
    m_hasCurrent = true;
    m_shown = 0.0f;
    m_pause = 0.25f;           // beat of silence so the eye lands before the text starts
    m_lastTickIdx = -1;
    m_sinceLine = 0.0f;
    // Character recognition: a signature motif whenever a different person starts talking.
    if (m_audio && (!m_hasLast || m_lastSpeaker != m_current.speaker)) m_audio->playSpeakerMotif(m_current.speaker);
    m_lastSpeaker = m_current.speaker;
    m_hasLast = true;
    if (m_onLineStart) m_onLineStart(m_lineCounter);
    ++m_lineCounter;
}

bool DialogueBox::advance() {
    if (!m_hasCurrent) return false;
    if (typing()) {
        m_shown = static_cast<float>(m_current.text.size());
        m_pause = 0.0f;
        if (m_audio) m_audio->playUi(2);
        return true;
    }
    if (m_audio) m_audio->playUi(2);
    startNext();
    return true;
}

void DialogueBox::update(float dt) {
    if (!m_hasCurrent) return;
    m_sinceLine += dt;
    if (!typing()) return;
    if (m_pause > 0.0f) {
        m_pause -= dt;
        return;
    }
    float before = m_shown;
    m_shown = std::min(static_cast<float>(m_current.text.size()), m_shown + dt * m_cps);
    int idxBefore = static_cast<int>(before), idxNow = static_cast<int>(m_shown);
    for (int i = idxBefore; i < idxNow; ++i) {
        QChar c = m_current.text[i];
        if (!c.isSpace() && m_audio && (i % 2 == 0) && i != m_lastTickIdx) {
            m_audio->playTypeTick(m_current.speaker);
            m_lastTickIdx = i;
        }
        if (isPausePunct(c) && i + 1 < m_current.text.size() && m_current.text[i + 1].isSpace()) {
            m_shown = static_cast<float>(i + 1);
            m_pause = pauseFor(c);
            break;
        }
    }
}

std::vector<QString> DialogueBox::wrap(const QString& text, const QFontMetrics& fm, int width) const {
    std::vector<QString> lines;
    QString cur;
    const QStringList words = text.split(' ');
    for (const QString& w : words) {
        QString trial = cur.isEmpty() ? w : cur + ' ' + w;
        if (!cur.isEmpty() && fm.horizontalAdvance(trial) > width) {
            lines.push_back(cur);
            cur = w;
        } else {
            cur = trial;
        }
    }
    if (!cur.isEmpty()) lines.push_back(cur);
    return lines;
}

void DialogueBox::render(QPainter& p, const QRect& b, float time) const {
    if (!m_hasCurrent) return;
    const Style& st = styleFor(m_current.speaker);
    const float s = std::max(0.6f, b.height() / 720.0f);

    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const int panelW = std::min(static_cast<int>(b.width() - 48 * s), static_cast<int>(1000 * s));
    const int panelH = static_cast<int>(140 * s);
    const int px = b.left() + (b.width() - panelW) / 2;
    const int py = b.bottom() - panelH - static_cast<int>(40 * s);

    // Panel rises in over the first 0.25s of every line.
    float rise = std::min(1.0f, m_sinceLine / 0.25f);
    float alpha = rise;
    p.setOpacity(alpha);
    p.translate(0, (1.0f - rise) * 12.0f * s);

    QRectF panel(px, py, panelW, panelH);
    QLinearGradient bg(panel.topLeft(), panel.bottomLeft());
    bg.setColorAt(0.0, QColor(10, 10, 14, 232));
    bg.setColorAt(1.0, QColor(18, 14, 18, 240));
    p.setPen(QPen(QColor(st.color.red(), st.color.green(), st.color.blue(), 150), 1.6));
    p.setBrush(bg);
    p.drawRoundedRect(panel, 8 * s, 8 * s);
    p.setPen(QPen(QColor(st.color.red(), st.color.green(), st.color.blue(), 45), 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(panel.adjusted(5 * s, 5 * s, -5 * s, -5 * s), 6 * s, 6 * s);

    // Name plate with emblem (the visual half of character recognition).
    const QString name = QString::fromUtf8(st.name);
    const QFont& nameFont = cachedFont(st.family, static_cast<int>(17 * s), true, false, 3.0 * s);
    QFontMetrics nfm(nameFont);
    if (!name.isEmpty()) {
        int plateW = nfm.horizontalAdvance(name) + static_cast<int>(70 * s);
        QRectF plate(px + 26 * s, py - 19 * s, plateW, 36 * s);
        p.setPen(QPen(st.color, 1.4));
        p.setBrush(QColor(12, 12, 16, 245));
        p.drawRoundedRect(plate, 6 * s, 6 * s);

        QPointF ic(plate.left() + 22 * s, plate.center().y());
        p.setPen(Qt::NoPen);
        p.setBrush(st.color);
        switch (m_current.speaker) {
        case Speaker::Nora: {                         // lantern
            p.drawRoundedRect(QRectF(ic.x() - 5 * s, ic.y() - 6 * s, 10 * s, 12 * s), 2 * s, 2 * s);
            p.drawRect(QRectF(ic.x() - 3 * s, ic.y() - 10 * s, 6 * s, 3 * s));
            break;
        }
        case Speaker::Mae: {                          // ribbon bow
            QPainterPath bow;
            bow.moveTo(ic); bow.lineTo(ic.x() - 9 * s, ic.y() - 6 * s); bow.lineTo(ic.x() - 9 * s, ic.y() + 6 * s); bow.closeSubpath();
            bow.moveTo(ic); bow.lineTo(ic.x() + 9 * s, ic.y() - 6 * s); bow.lineTo(ic.x() + 9 * s, ic.y() + 6 * s); bow.closeSubpath();
            p.drawPath(bow);
            break;
        }
        case Speaker::Crane: {                        // key
            p.drawEllipse(QPointF(ic.x() - 5 * s, ic.y()), 4.5 * s, 4.5 * s);
            p.drawRect(QRectF(ic.x(), ic.y() - 1.2 * s, 10 * s, 2.4 * s));
            p.drawRect(QRectF(ic.x() + 7 * s, ic.y(), 2 * s, 4 * s));
            break;
        }
        case Speaker::Ada: {                          // teardrop
            QPainterPath d;
            d.moveTo(ic.x(), ic.y() - 8 * s); d.quadTo(ic.x() + 8 * s, ic.y() + 2 * s, ic.x(), ic.y() + 7 * s);
            d.quadTo(ic.x() - 8 * s, ic.y() + 2 * s, ic.x(), ic.y() - 8 * s);
            p.drawPath(d);
            break;
        }
        case Speaker::Eli:                            // building block
            p.drawRect(QRectF(ic.x() - 6 * s, ic.y() - 6 * s, 12 * s, 12 * s));
            p.setBrush(QColor(12, 12, 16));
            p.drawEllipse(ic, 2.5 * s, 2.5 * s);
            break;
        default:                                      // house window
            p.drawRect(QRectF(ic.x() - 6 * s, ic.y() - 7 * s, 12 * s, 14 * s));
            p.setBrush(QColor(12, 12, 16));
            p.drawRect(QRectF(ic.x() - 0.8 * s, ic.y() - 7 * s, 1.6 * s, 14 * s));
            p.drawRect(QRectF(ic.x() - 6 * s, ic.y() - 0.8 * s, 12 * s, 1.6 * s));
            break;
        }
        p.setFont(nameFont);
        p.setPen(st.color);
        p.drawText(QRectF(ic.x() + 18 * s, plate.top(), plateW, plate.height()), Qt::AlignVCenter | Qt::AlignLeft, name);
    }

    // Body text, wrapped for the *full* sentence so words never jump lines while typing.
    const QFont& textFont = cachedFont(st.family, static_cast<int>((m_current.speaker == Speaker::Crane ? 23 : 26) * s), false, st.italic);
    QFontMetrics fm(textFont);
    const int textX = px + static_cast<int>(34 * s);
    const int textW = panelW - static_cast<int>(68 * s);
    const auto lines = wrap(m_current.text, fm, textW);
    p.setFont(textFont);
    int shown = static_cast<int>(m_shown);
    int y = py + static_cast<int>(54 * s);
    const int lineH = static_cast<int>(fm.height() * 1.12f);
    int consumed = 0;
    QPointF caret;
    bool caretSet = false;
    for (const QString& ln : lines) {
        int take = std::clamp(shown - consumed, 0, static_cast<int>(ln.size()));
        QString vis = ln.left(take);
        p.setPen(QColor(st.color.red() * 0.35 + 255 * 0.65, st.color.green() * 0.35 + 255 * 0.65, st.color.blue() * 0.35 + 255 * 0.65));
        p.drawText(textX, y, vis);
        if (!caretSet && take < ln.size()) {
            caret = QPointF(textX + fm.horizontalAdvance(vis), y);
            caretSet = true;
        }
        consumed += ln.size() + 1;   // +1 for the space the wrap swallowed
        y += lineH;
    }
    if (typing() && (static_cast<int>(time * 2.5f) & 1)) {
        if (!caretSet) caret = QPointF(textX, py + 54 * s);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(st.color.red(), st.color.green(), st.color.blue(), 200));
        p.drawRect(QRectF(caret.x() + 2 * s, caret.y() - fm.ascent() * 0.8, 2 * s, fm.ascent() * 0.9));
    }

    // "Click to continue" cue: bouncing triangle once the line is fully typed.
    if (lineComplete()) {
        float bob = std::sin(time * 5.0f) * 3.0f * s;
        float a = 0.55f + 0.45f * std::sin(time * 3.0f);
        QPointF c(panel.right() - 34 * s, panel.bottom() - 26 * s + bob);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(st.color.red(), st.color.green(), st.color.blue(), static_cast<int>(255 * a)));
        QPainterPath tri;
        tri.moveTo(c.x() - 8 * s, c.y() - 5 * s);
        tri.lineTo(c.x() + 8 * s, c.y() - 5 * s);
        tri.lineTo(c.x(), c.y() + 7 * s);
        tri.closeSubpath();
        p.drawPath(tri);
        p.setFont(cachedFont(st.family, static_cast<int>(12 * s)));
        p.setPen(QColor(160, 160, 170, static_cast<int>(180 * a)));
        p.drawText(QRectF(panel.right() - 240 * s, panel.bottom() - 28 * s, 170 * s, 20 * s), Qt::AlignRight | Qt::AlignVCenter, "click to continue");
    }
    p.restore();
}

} // namespace hm
