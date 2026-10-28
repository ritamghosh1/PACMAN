#pragma once

#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QString>
#include <vector>

namespace hm {

enum class StoryAction { None, Back, WatchIntro, WatchEnding };

// Lab-demo screen: the full backstory (what the Quiet Programme was, why Nora and Mae are involved),
// a clickable gallery of the five ghosts with their histories, how the game plays, and every ending
// explained, so the whole concept can be shown without finishing the game.
class StoryBrowser {
public:
    StoryBrowser();

    void init(const QRect& bounds);
    void update(float dt);
    void render(QPainter& p, const QRect& bounds, float time);

    void onMouseMove(const QPoint& pos);
    StoryAction onClick(const QPoint& pos);   // sets endingIndex() when WatchEnding
    void onWheel(int delta);
    int endingIndex() const { return m_endingToWatch; }
    int hoveredItem() const { return m_hover; }
    void reset();

private:
    struct Item { QString label; QRect rect; };
    struct Page { QString title; QString subtitle; QString body; int ending = -1; bool ghosts = false; };
    struct GhostCard { QString name, alias, subtitle, backstory, counter, sound; };
    void select(int index);
    QRect watchButton() const;
    QRect ghostCardRect(int i) const;
    void drawGhostGallery(QPainter& p, float time);
    void drawBody(QPainter& p, const QString& text, const QRect& area);

    std::vector<Item> m_items;
    std::vector<Page> m_pages;
    std::vector<GhostCard> m_ghosts;
    QRect m_content;
    int m_selected = 0;
    int m_hover = -1;
    int m_ghostSel = 0;
    int m_ghostHover = -1;
    float m_typed = 0.0f;
    float m_scroll = 0.0f;
    float m_maxScroll = 0.0f;
    int m_endingToWatch = 0;
    bool m_hoverWatch = false;
};

} // namespace hm
