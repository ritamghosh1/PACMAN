#include "GLView.h"
#include "core/Input.h"
#include "story/Subtitles.h"
#include "save/SaveGame.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QCursor>
#include <QApplication>

namespace hm {

GLView::GLView(QWidget* parent)
    : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_settings.loadFromFile();

    connect(&m_frameTimer, &QTimer::timeout, this, [this]() {
        update();
    });
    m_frameTimer.start(16); // ~60 FPS update
}

GLView::~GLView() {
    makeCurrent();
    m_renderer.destroy(this);
    doneCurrent();
}

void GLView::setCursorCaptured(bool captured) {
    m_cursorCaptured = captured;
    Input::instance().setMouseLocked(captured);
    if (captured) {
        setCursor(Qt::BlankCursor);
        QPoint center(width() / 2, height() / 2);
        QCursor::setPos(mapToGlobal(center));
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void GLView::setGameState(GameState state) {
    m_state = state;
    if (m_state == GameState::Playing) {
        setCursorCaptured(true);
    } else {
        setCursorCaptured(false);
    }

    if (m_state == GameState::Intro) {
        if (!m_introMovie) {
            m_introMovie = std::make_unique<IntroMovie>(&m_world.audio());
        }
        m_introMovie->reset();
    } else if (m_state == GameState::Ending) {
        EndingType ending = EndingResolver::resolve(m_world.choices());
        m_endingCutscene = std::make_unique<EndingCutscene>(ending, &m_world.audio());
    }
}

void GLView::initializeGL() {
    initializeOpenGLFunctions();

    const GLubyte* glRenderer = glGetString(GL_RENDERER);
    const GLubyte* glVersion = glGetString(GL_VERSION);
    qInfo("Hollowmere Engine initialized on: %s (OpenGL %s)", glRenderer, glVersion);

    int pixelW = static_cast<int>(width() * devicePixelRatioF());
    int pixelH = static_cast<int>(height() * devicePixelRatioF());

    m_renderer.init(this, pixelW, pixelH);
    m_world.init(this);

    m_menuOverlay.init(rect());
    m_pauseOverlay.init(rect());
    m_journalOverlay.init(rect());
    m_inventoryOverlay.init(rect());

    m_clock.reset();
    setGameState(GameState::Menu);
}

void GLView::resizeGL(int w, int h) {
    int pixelW = static_cast<int>(w * devicePixelRatioF());
    int pixelH = static_cast<int>(h * devicePixelRatioF());
    m_renderer.resize(this, pixelW, pixelH);

    m_menuOverlay.init(rect());
    m_pauseOverlay.init(rect());
    m_journalOverlay.init(rect());
    m_inventoryOverlay.init(rect());
}

void GLView::paintGL() {
    m_clock.tick();
    float dt = static_cast<float>(m_clock.frameTime());
    m_totalTime += dt;

    if (m_state == GameState::Menu) {
        m_menuOverlay.update(dt);

        // Render 3D foyer live background
        m_renderer.render(this, m_world, m_totalTime);

        QPainter painter(this);
        m_menuOverlay.render(painter, rect(), QFile::exists("save.json"));
        painter.end();
        return;
    }

    if (m_state == GameState::Intro) {
        if (m_introMovie) {
            bool spaceDown = Input::instance().isKeyDown(Qt::Key_Space);
            m_introMovie->update(dt, spaceDown);

            QPainter painter(this);
            m_introMovie->render(painter, rect());
            drawSubtitles(painter, rect());
            painter.end();

            if (m_introMovie->isFinished()) {
                setGameState(GameState::Playing);
            }
        }
        return;
    }

    if (m_state == GameState::Ending) {
        if (m_endingCutscene) {
            bool spaceDown = Input::instance().isKeyDown(Qt::Key_Space);
            m_endingCutscene->update(dt, spaceDown);

            QPainter painter(this);
            m_endingCutscene->render(painter, rect());
            painter.end();

            if (m_endingCutscene->isFinished()) {
                setGameState(GameState::Menu);
            }
        }
        return;
    }

    // GameState::Playing or Paused
    if (m_state == GameState::Playing) {
        while (m_clock.hasStep()) {
            m_world.update(static_cast<float>(m_clock.deltaSim()));
            m_clock.consumeStep();
        }

        bool hintHeld = Input::instance().isKeyDown(Qt::Key_H);
        m_journalOverlay.update(dt, hintHeld);

        // Check if player reached all 5 sigils and entered the chapel crypt finale
        if (m_world.player().sigilsCollected() >= 5 && m_world.player().position().z > 23.0f) {
            setGameState(GameState::Ending);
            return;
        }
    }

    // Reset single-frame input deltas
    Input::instance().update();

    // 2. 3D Render Pass
    m_renderer.render(this, m_world, m_totalTime);

    // 3. 2D HUD / Overlays Pass
    QPainter painter(this);

    if (m_world.deathSequence().isActive()) {
        float blackAlpha = m_world.deathSequence().blackOverlayAlpha();
        if (blackAlpha > 0.0f) {
            painter.fillRect(rect(), QColor(0, 0, 0, static_cast<int>(blackAlpha * 255)));
        }
    }

    if (m_state == GameState::Playing) {
        if (m_settings.qualityLevel >= 0) { // Render HUD
            m_hud.draw(painter, rect(), m_world, static_cast<float>(m_clock.fps()));
        }

        if (m_inventoryOpen) {
            m_inventoryOverlay.render(painter, rect(), m_world.inventory());
        }

        if (m_journalOpen) {
            m_journalOverlay.render(painter, rect(), m_world.journal(), m_world.activeWing());
        }

        if (m_world.mercy().isActive()) {
            m_rhythmOverlay.render(painter, rect(), m_world.mercy());
        }

        m_debugConsole.render(painter, rect(), m_world, static_cast<float>(m_clock.fps()));
    } else if (m_state == GameState::Paused) {
        m_pauseOverlay.render(painter, rect());
    }

    // Subtitles on top
    if (m_settings.subtitles) {
        drawSubtitles(painter, rect());
    }

    painter.end();
}

void GLView::drawSubtitles(QPainter& p, const QRect& bounds) {
    auto& subs = SubtitleSystem::instance();
    if (!subs.hasActive()) return;

    float alpha = subs.alpha();
    if (alpha <= 0.01f) return;

    int cx = bounds.width() / 2;
    int cy = bounds.height() - 75;

    QString fullText = subs.currentSpeaker().isEmpty() ? subs.currentText() : QString("%1: %2").arg(subs.currentSpeaker()).arg(subs.currentText());

    QFont subFont("Georgia", 13, QFont::Normal);
    p.setFont(subFont);

    QFontMetrics fm(subFont);
    int textW = fm.horizontalAdvance(fullText) + 32;
    int textH = 34;

    QRect bgRect(cx - textW / 2, cy - textH / 2, textW, textH);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, static_cast<int>(180 * alpha)));
    p.drawRoundedRect(bgRect, 4, 4);

    QColor col = subs.currentColor();
    col.setAlpha(static_cast<int>(255 * alpha));
    p.setPen(col);
    p.drawText(bgRect, Qt::AlignCenter, fullText);
}

void GLView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        if (m_state == GameState::Playing) {
            if (m_journalOpen) {
                m_journalOpen = false;
            } else if (m_inventoryOpen) {
                m_inventoryOpen = false;
            } else {
                setGameState(GameState::Paused);
            }
            return;
        } else if (m_state == GameState::Paused) {
            setGameState(GameState::Playing);
            return;
        }
    }

    if (m_state == GameState::Playing) {
        if (event->key() == Qt::Key_Tab) {
            m_journalOpen = !m_journalOpen;
            return;
        }
        if (event->key() == Qt::Key_I) {
            m_inventoryOpen = !m_inventoryOpen;
            return;
        }
        if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_6) {
            int slot = event->key() - Qt::Key_1;
            m_inventoryOverlay.selectSlot(slot);
            return;
        }

        // Debug shortcuts
        if (event->key() == Qt::Key_F3) {
            m_debugConsole.toggleDebugOverlay();
            return;
        }
        if (event->key() == Qt::Key_F4) {
            m_debugConsole.toggleNoclip();
            return;
        }
        if (event->key() == Qt::Key_F5) {
            m_debugConsole.giveAllItems(m_world);
            return;
        }

        if (m_journalOpen) {
            if (event->key() == Qt::Key_Up) {
                m_journalOverlay.prevNote();
                return;
            } else if (event->key() == Qt::Key_Down) {
                m_journalOverlay.nextNote();
                return;
            }
        }
    }

    Input::instance().onKeyPressed(event->key());
    event->accept();
}

void GLView::keyReleaseEvent(QKeyEvent* event) {
    Input::instance().onKeyReleased(event->key());
    event->accept();
}

void GLView::mouseMoveEvent(QMouseEvent* event) {
    if (m_state == GameState::Menu) {
        m_menuOverlay.onMouseMove(event->pos());
        return;
    }
    if (m_state == GameState::Paused) {
        m_pauseOverlay.onMouseMove(event->pos());
        return;
    }

    if (!m_cursorCaptured) return;

    QPoint center(width() / 2, height() / 2);
    int dx = event->pos().x() - center.x();
    int dy = event->pos().y() - center.y();

    if (dx != 0 || dy != 0) {
        Input::instance().onMouseMove(dx, dy);
        QCursor::setPos(mapToGlobal(center));
    }
    event->accept();
}

void GLView::mousePressEvent(QMouseEvent* event) {
    if (m_state == GameState::Menu) {
        MenuAction act = m_menuOverlay.onMouseClick(event->pos());
        if (act == MenuAction::NewGame) {
            setGameState(GameState::Intro);
        } else if (act == MenuAction::Continue) {
            SaveData data;
            if (SaveGame::loadFromFile("save.json", data)) {
                m_world.player().spawn(data.playerPos);
                m_world.player().refillOil(data.lanternOil);
                m_world.sanity().setSanity(data.sanity);
            }
            setGameState(GameState::Playing);
        } else if (act == MenuAction::Settings) {
            SettingsDialog dlg(m_settings, this);
            dlg.exec();
        } else if (act == MenuAction::Credits) {
            m_world.choices().sparedAda = true;
            m_world.choices().sparedEli = true;
            setGameState(GameState::Ending);
        } else if (act == MenuAction::Quit) {
            QApplication::quit();
        }
        return;
    }

    if (m_state == GameState::Paused) {
        PauseAction act = m_pauseOverlay.onMouseClick(event->pos());
        if (act == PauseAction::Resume) {
            setGameState(GameState::Playing);
        } else if (act == PauseAction::Settings) {
            SettingsDialog dlg(m_settings, this);
            dlg.exec();
        } else if (act == PauseAction::Save) {
            SaveData data;
            data.playerPos = m_world.player().position();
            data.playerYaw = m_world.player().yaw();
            data.playerPitch = m_world.player().pitch();
            data.lanternOil = m_world.player().lantern().oil();
            data.sanity = m_world.sanity().sanity();
            SaveGame::saveToFile("save.json", data);
            SubtitleSystem::instance().show("Hollowmere", "Game checkpoint saved.", 2.5f);
            setGameState(GameState::Playing);
        } else if (act == PauseAction::LoadLastCheckpoint) {
            SaveData data;
            if (SaveGame::loadFromFile("save.json", data)) {
                m_world.player().spawn(data.playerPos);
                m_world.player().refillOil(data.lanternOil);
                m_world.sanity().setSanity(data.sanity);
                SubtitleSystem::instance().show("Hollowmere", "Checkpoint loaded.", 2.5f);
            }
            setGameState(GameState::Playing);
        } else if (act == PauseAction::QuitToMenu) {
            setGameState(GameState::Menu);
        }
        return;
    }

    if (!m_cursorCaptured) {
        setCursorCaptured(true);
        return;
    }
    Input::instance().onMouseButtonPressed(event->button());
    event->accept();
}

void GLView::mouseReleaseEvent(QMouseEvent* event) {
    Input::instance().onMouseButtonReleased(event->button());
    event->accept();
}

void GLView::focusOutEvent(QFocusEvent* event) {
    Input::instance().onFocusLost();
    if (m_state == GameState::Playing) {
        setGameState(GameState::Paused);
    }
    QOpenGLWidget::focusOutEvent(event);
}

} // namespace hm
