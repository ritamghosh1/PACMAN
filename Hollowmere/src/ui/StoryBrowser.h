#pragma once

#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QString>
#include <vector>

namespace hm {

enum class StoryAction { None, Back, WatchIntro, WatchEnding };

// Lab-demo screen: the premise, how the game plays, and all five endings with how to reach each,
// so the whole concept can be shown without finishing the game.
class StoryBrowser {
public:
    StoryBrowser();

    void init(const QRect& bounds);
    void update(float dt);
    void render(QPainter& p, const QRect& bounds, float time);

    void onMouseMove(const QPoint& pos);
    StoryAction onClick(const QPoint& pos);   // sets endingIndex() when WatchEnding
    int endingIndex() const { return m_endingToWatch; }
    int hoveredItem() const { return m_hover; }
    void reset();

private:
    struct Item { QString label; QRect rect; };
    struct Page { QString title; QString subtitle; QString body; int ending = -1; };
    void select(int index);
    QRect watchButton() const;

    std::vector<Item> m_items;
    std::vector<Page> m_pages;
    QRect m_content;
    int m_selected = 0;
    int m_hover = -1;
    float m_typed = 0.0f;
    int m_endingToWatch = 0;
    bool m_hoverWatch = false;
};

} // namespace hm
