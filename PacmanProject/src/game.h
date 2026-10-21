#pragma once

#include <QColor>
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
    enum class S { Menu, Ready, Playing, Dying, Won, Intermission, Over };

    // Single  : one Pac-Man (arrows or WASD).
    // Coop    : two Pac-Men share the maze (P1 arrows, P2 WASD), separate scores/lives.
    // Versus  : P1 is Pac-Man (arrows), P2 drives the red ghost Blinky (WASD).
    enum class Mode { Single = 0, Coop = 1, Versus = 2 };

    struct Player {
        Entity pac;
        int score = 0;
        int lives = 3;
        bool extraAwarded = false;
        QColor color;
        double deathT = 0;    // co-op: death animation time left (player frozen)
        double respawnT = 0;  // co-op: post-respawn grace (blinking, ghosts pass through)
        bool active() const { return lives > 0; }
    };

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
    void spawnPlayer(int i);
    void updatePlaying(double dt);
    void triggerFright();
    void advanceMode();
    void addScore(int player, int n);
    void startNextLevel();
    void renderScene();
    void renderIntermission();
    void renderMenu();
    void drawHud();
    void drawCentered(int y, const QString& text, const QColor& color);

    bool humanGhost(const Ghost& g) const { return mode_ == Mode::Versus && g.id == 0; }
    const Entity& nearestPac(const Entity& from) const;
    bool ghostPass(const Ghost& g, int r, int c) const;
    Dir pacDecide(Entity& e);
    Dir ghostDecide(Entity& e);
    QPoint ghostTarget(const Ghost& g) const;
    Dir bfsStep(const Ghost& g, QPoint target) const;

    Maze maze_;
    std::vector<Player> players_;
    std::vector<Ghost> ghosts_;
    std::vector<ScorePopup> popups_;
    SoundManager sound_;
    QImage buffer_;

    Mode mode_ = Mode::Single;
    int menuSel_ = 0;
    int dyingIdx_ = 0;     // which player is playing the death animation
    int winner_ = -1;      // Versus result: 0 = Pac-Man, 1 = Ghost
    int ghostScore_ = 0;   // Versus: points earned by the ghost player

    static constexpr double kDeathSecs = 1.5;
    static constexpr double kRespawnGraceSecs = 2.5;

    S st_ = S::Menu;
    bool paused_ = false;
    double stateT_ = 0;    // current state countdown
    double frightT_ = 0;   // frightened mode remaining
    double modeT_ = 0;     // scatter/chase segment remaining
    double animT_ = 0;
    int frightChain_ = 0;
    int modeIdx_ = 0;
    bool chase_ = false;
    int highScore_ = 10000;
    int level_ = 1;
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
