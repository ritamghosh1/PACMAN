#pragma once

#include <QElapsedTimer>
#include <QImage>
#include <QTimer>
#include <QWidget>
#include <vector>

#include "ghost.h"
#include "maze.h"
#include "sound.h"

class Game : public QWidget {
    Q_OBJECT
public:
    explicit Game(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    enum class S { Ready, Playing, Dying, Won, Over };

    struct ScorePopup {
        int x;
        int y;
        int points;
        double timer;
    };

    void tick();
    void newGame();
    void applyTuning();
    void resetActors();
    void updatePlaying(double dt);
    void triggerFright();
    void advanceMode();
    void addScore(int n);
    void spawnFruit();
    void renderScene();

    bool ghostPass(const Ghost& g, int r, int c) const;
    Dir pacDecide(Entity& e);
    Dir ghostDecide(Entity& e);
    QPoint ghostTarget(const Ghost& g) const;
    Dir bfsStep(const Ghost& g, QPoint target) const;

    Maze maze_;
    Entity pac_;
    std::vector<Ghost> ghosts_;
    std::vector<ScorePopup> popups_;
    SoundManager sound_;
    QImage buffer_;

    S st_ = S::Ready;
    bool paused_ = false;
    double stateT_ = 0;    // current state countdown
    double frightT_ = 0;   // frightened mode remaining
    double modeT_ = 0;     // scatter/chase segment remaining
    double animT_ = 0;
    int frightChain_ = 0;
    int modeIdx_ = 0;
    bool chase_ = false;
    int score_ = 0;
    int highScore_ = 10000;
    int lives_ = 3;
    int level_ = 1;
    bool extraAwarded_ = false;
    int dotsEatenCount_ = 0;
    bool fruitActive_ = false;
    double fruitT_ = 0;

    double pacSpeed_ = 7.0;
    double ghostSpeed_ = 5.5;
    double frightSecs_ = 6.0;
    double releaseBase_ = 1.6;
    int predictive_ = 0;
    QElapsedTimer clock_;
    QTimer* timer_ = nullptr;
};
