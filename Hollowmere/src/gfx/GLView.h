#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QTimer>
#include <memory>
#include "Renderer.h"
#include "world/World.h"
#include "core/Clock.h"
#include "ui/Hud.h"
#include "ui/MenuWidget.h"
#include "ui/PauseMenu.h"
#include "ui/JournalWidget.h"
#include "ui/InventoryWidget.h"
#include "ui/RhythmWidget.h"
#include "ui/SettingsDialog.h"
#include "app/GameState.h"
#include "app/DebugConsole.h"
#include "story/IntroMovie.h"
#include "story/Ending.h"
#include "Overlay2D.h"

namespace hm {

class GLView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit GLView(QWidget* parent = nullptr);
    ~GLView() override;

    void setCursorCaptured(bool captured);
    bool isCursorCaptured() const { return m_cursorCaptured; }

    GameState gameState() const { return m_state; }
    void setGameState(GameState state);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    Renderer m_renderer;
    World m_world;
    Clock m_clock;
    Hud m_hud;

    GameState m_state = GameState::Menu;
    GameSettings m_settings;

    MenuOverlay m_menuOverlay;
    PauseOverlay m_pauseOverlay;
    JournalOverlay m_journalOverlay;
    InventoryOverlay m_inventoryOverlay;
    RhythmOverlay m_rhythmOverlay;
    DebugConsole m_debugConsole;
    Overlay2D m_overlay;

    std::unique_ptr<IntroMovie> m_introMovie;
    std::unique_ptr<EndingCutscene> m_endingCutscene;

    bool m_journalOpen = false;
    bool m_inventoryOpen = false;

    QTimer m_frameTimer;
    float m_totalTime = 0.0f;
    bool m_cursorCaptured = false;

    void drawSubtitles(QPainter& p, const QRect& bounds);
};

} // namespace hm
