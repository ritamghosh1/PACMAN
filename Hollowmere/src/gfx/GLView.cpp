#include "GLView.h"
#include "core/Input.h"
#include "ui/FontCache.h"
#include "story/Subtitles.h"
#include "save/SaveGame.h"
#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStandardPaths>
#include <cmath>

namespace hm {

namespace {
QString savePath() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/save.json";
}
} // namespace

GLView::GLView(QWidget* parent)
    : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_settings.loadFromFile();
    cinesfx::Bank::prewarm();   // render all cutscene sounds on a worker thread while the menu is up

    connect(&m_frameTimer, &QTimer::timeout, this, [this]() { update(); });
    m_frameTimer.start(8);
}

GLView::~GLView() {
    makeCurrent();
    m_renderer.destroy(this);
    m_overlay.destroy(this);
    doneCurrent();
}

void GLView::setCursorCaptured(bool captured) {
    m_cursorCaptured = captured;
    Input::instance().setMouseLocked(captured);
    if (captured) {
        setCursor(Qt::BlankCursor);
        m_ignoreNextMove = true;
        QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void GLView::applySettings() {
    m_world.audio().setMasterVolume(m_settings.masterVolume);
    m_world.player().setMouseSensitivity(m_settings.mouseSensitivity);
    m_world.player().setInvertY(m_settings.invertY);
    m_renderer.camera().setFov(m_settings.fov);
}

void GLView::startMemory(int sigil) {
    m_cinematic = makeMemoryCinematic(sigil, &m_world.audio());
    m_cinematic->start();
    m_returnToStory = false;
    setGameState(GameState::Memory);
}

void GLView::openStory() {
    m_storyBrowser.init(rect());
    m_storyBrowser.reset();
    setGameState(GameState::Story);
}

void GLView::startStoryMovie(bool intro, int ending) {
    m_world.audio().stopBed(104, 1.0f);
    m_world.audio().stopBed(105, 1.0f);
    m_cinematic = intro ? makeIntroCinematic(&m_world.audio()) : makeEndingCinematic(static_cast<EndingType>(ending), &m_world.audio());
    m_cinematic->start();
    m_returnToStory = true;
    setGameState(intro ? GameState::Intro : GameState::Ending);
}

void GLView::drawPauseRecap(QPainter& p) {
    p.save();
    QRect box(40, 60, 420, rect().height() - 120);
    p.setPen(QPen(QColor(235, 205, 140, 90), 1.2));
    p.setBrush(QColor(8, 8, 12, 215));
    p.drawRoundedRect(box, 8, 8);
    p.setPen(QColor(235, 205, 140));
    p.setFont(cachedFont("Georgia", 15, true, false, 3.0));
    p.drawText(box.adjusted(18, 14, -18, 0), Qt::AlignTop | Qt::AlignLeft, "THE STORY SO FAR");
    p.setPen(QColor(222, 216, 206));
    p.setFont(cachedFont("Georgia", 16));
    p.drawText(box.adjusted(18, 46, -18, -14), Qt::TextWordWrap | Qt::AlignTop | Qt::AlignLeft, m_world.storyRecap());
    p.restore();
}

void GLView::humTap() {
    int r = m_world.mercy().tap();
    static const std::vector<int16_t> note = cinesfx::musicBoxNote(880.0f, 1.2f);
    if (r == 1) m_world.audio().playCine(note, 0.8f);
    else if (r == 2) m_world.audio().playUi(1);
    if (m_world.mercy().isAdaPacified()) m_world.queueStory(Speaker::Ada, "...Nora? You kept the song. Thank you for staying.");
}

void GLView::applyQuality() {
    makeCurrent();   // quality changes reallocate GPU render targets
    applySettings();
    m_renderer.setQuality(this, m_settings.qualityLevel);
    doneCurrent();
}

void GLView::startMenuAmbience() {
    if (m_menuAmbienceOn || !cinesfx::Bank::ready()) return;
    const auto& b = cinesfx::Bank::get();
    m_world.audio().startBed(104, b.wind, 0.30f, 3.0f);
    m_world.audio().startBed(105, b.padLow, 0.32f, 4.0f);
    m_menuAmbienceOn = true;
}

void GLView::setGameState(GameState state) {
    GameState prev = m_state;
    m_state = state;
    setCursorCaptured(state == GameState::Playing);
    if (state == GameState::Story && prev != GameState::Menu) m_menuAmbienceOn = false;   // beds were stopped by a movie
    if (state == GameState::Menu || state == GameState::Story) {
        if (state == GameState::Menu) m_cinematic.reset();
        if (prev != GameState::Menu) m_menuAmbienceOn = false;   // re-arm so the beds restart
        m_world.audio().setGameplayMuted(false);
    }
    if (state == GameState::Playing && prev != GameState::Paused) {
        m_world.audio().stopBed(104, 2.0f);
        m_world.audio().stopBed(105, 2.0f);
        m_menuAmbienceOn = false;
    }
    Input::instance().update();
}

void GLView::initializeGL() {
    initializeOpenGLFunctions();
    qInfo("Hollowmere Engine initialized on: %s (OpenGL %s)", glGetString(GL_RENDERER), glGetString(GL_VERSION));

    int pixelW = static_cast<int>(width() * devicePixelRatioF());
    int pixelH = static_cast<int>(height() * devicePixelRatioF());

    m_renderer.init(this, pixelW, pixelH);
    m_overlay.init(this, width(), height(), 1.0);   // HUD is rasterised at logical size (4x cheaper on Retina) and scaled up by the GPU
    m_world.init(this);
    m_dialogue.setAudio(&m_world.audio());

    m_menuOverlay.init(rect());
    m_pauseOverlay.init(rect());
    m_journalOverlay.init(rect());
    m_inventoryOverlay.init(rect());
    applySettings();
    m_renderer.setQuality(this, m_settings.qualityLevel);

    m_clock.reset();
    setGameState(GameState::Menu);
}

void GLView::resizeGL(int w, int h) {
    int pixelW = static_cast<int>(w * devicePixelRatioF());
    int pixelH = static_cast<int>(h * devicePixelRatioF());
    m_renderer.resize(this, pixelW, pixelH);
    m_overlay.resize(this, w, h, 1.0);
    m_menuOverlay.init(rect());
    m_pauseOverlay.init(rect());
    m_journalOverlay.init(rect());
    m_inventoryOverlay.init(rect());
}

void GLView::startNewGame() {
    m_world.audio().playUi(1);
    m_world.audio().stopBed(104, 1.5f);
    m_world.audio().stopBed(105, 1.5f);
    m_cinematic = makeIntroCinematic(&m_world.audio());
    m_cinematic->start();
    m_cinematicIsEnding = false;
    setGameState(GameState::Intro);
}

void GLView::finishIntro() {
    m_cinematic.reset();
    m_world.newGame();           // fresh manor + the first-steps briefing
    applySettings();
    m_hud.resetSession();
    m_dialogue.clear();
    m_journalOpen = m_inventoryOpen = false;
    setGameState(GameState::Playing);
}

void GLView::beginEnding() {
    m_world.clearEndingRequest();
    EndingType ending = EndingResolver::resolve(m_world.choices());
    m_cinematic = makeEndingCinematic(ending, &m_world.audio());
    m_cinematic->start();
    m_cinematicIsEnding = true;
    m_returnToStory = false;
    setGameState(GameState::Ending);
}

void GLView::paintCinematic(float dt, Cinematic* cine) {
    int pixelW = static_cast<int>(width() * devicePixelRatioF());
    int pixelH = static_cast<int>(height() * devicePixelRatioF());
    GLuint targetFbo = defaultFramebufferObject();
    cine->update(dt);
    glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
    glViewport(0, 0, pixelW, pixelH);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (QPainter* p = m_overlay.begin()) {
        cine->render(*p, rect());
        m_overlay.end();
        m_overlay.render(this, targetFbo, pixelW, pixelH);
    }
    Input::instance().update();
}

void GLView::stepWorld(float dt) {
    bool frozen = m_dialogue.active() || m_journalOpen || m_inventoryOpen;
    while (m_clock.hasStep()) {
        if (!frozen) {
            m_world.update(static_cast<float>(m_clock.deltaSim()));
            // Edge-triggered input (E, F, clicks, mouse delta) must be consumed once per step,
            // otherwise a 60 fps frame running two 120 Hz steps would toggle a door twice.
            Input::instance().update();
        }
        m_clock.consumeStep();
    }
    if (frozen) Input::instance().update();

    auto story = m_world.takeStory();
    if (!story.empty()) m_dialogue.push(story);
    int wing = m_world.wingJustEntered();
    if (wing >= 0) m_hud.showLocation(m_world.map().wings[wing].name);
    (void)dt;
}

void GLView::paintGL() {
    m_clock.tick();
    float dt = std::min(0.1f, static_cast<float>(m_clock.frameTime()));
    m_totalTime += dt;

    int pixelW = static_cast<int>(width() * devicePixelRatioF());
    int pixelH = static_cast<int>(height() * devicePixelRatioF());
    GLuint targetFbo = defaultFramebufferObject();

    if (m_state == GameState::Menu) {
        startMenuAmbience();
        m_menuOverlay.update(dt);
        m_world.player().setYaw(-90.0f + std::sin(m_totalTime * 0.25f) * 24.0f);   // slow look-around behind the menu
        m_renderer.render(this, m_world, m_totalTime, targetFbo);
        if (QPainter* p = m_overlay.begin()) {
            m_menuOverlay.render(*p, rect(), QFile::exists(savePath()));
            m_overlay.end();
            m_overlay.render(this, targetFbo, pixelW, pixelH);
        }
        Input::instance().update();
        return;
    }

    if (m_state == GameState::Story) {
        startMenuAmbience();
        m_storyBrowser.update(dt);
        m_world.player().setYaw(-90.0f + std::sin(m_totalTime * 0.25f) * 24.0f);
        m_renderer.render(this, m_world, m_totalTime, targetFbo);
        if (QPainter* p = m_overlay.begin()) {
            m_storyBrowser.render(*p, rect(), m_totalTime);
            m_overlay.end();
            m_overlay.render(this, targetFbo, pixelW, pixelH);
        }
        Input::instance().update();
        return;
    }

    if (inCinematic()) {
        if (m_cinematic) {
            paintCinematic(dt, m_cinematic.get());
            if (m_cinematic->finished()) {
                if (m_returnToStory) { m_returnToStory = false; m_cinematic.reset(); m_storyBrowser.init(rect()); setGameState(GameState::Story); }
                else if (m_state == GameState::Intro) finishIntro();
                else if (m_state == GameState::Memory) { m_cinematic.reset(); setGameState(GameState::Playing); }
                else setGameState(GameState::Menu);
            }
        }
        return;
    }

    if (m_state == GameState::Playing) {
        stepWorld(dt);
        m_dialogue.update(dt);
        m_hud.update(dt);
        m_journalOverlay.update(dt, Input::instance().isKeyDown(Qt::Key_H));
        if (m_world.endingRequested()) {
            beginEnding();
            return;
        }
        if (int mem = m_world.takeMemory()) {
            startMemory(mem);
            return;
        }
    }

    m_renderer.render(this, m_world, m_totalTime, targetFbo);

    if (QPainter* p = m_overlay.begin()) {
        if (m_world.deathSequence().isActive()) {
            float a = m_world.deathSequence().blackOverlayAlpha();
            if (a > 0.0f) p->fillRect(rect(), QColor(0, 0, 0, static_cast<int>(a * 255)));
        }
        if (m_state == GameState::Playing) {
            float aspect = static_cast<float>(width()) / std::max(1, height());
            Mat4 viewProj = m_renderer.camera().projMatrix(aspect) * m_renderer.camera().viewMatrix();
            if (!m_world.deathSequence().isActive())
                m_hud.draw(*p, rect(), m_world, static_cast<float>(m_clock.fps()), viewProj, m_totalTime, m_debugConsole.showDebugOverlay());
            if (m_inventoryOpen) m_inventoryOverlay.render(*p, rect(), m_world.inventory());
            if (m_journalOpen) {
                m_journalOverlay.setObjective(m_world.objective().title + ". " + m_world.objective().detail);
                m_journalOverlay.render(*p, rect(), m_world.journal(), m_world.activeWing());
            }
            if (m_world.mercy().isActive()) m_rhythmOverlay.render(*p, rect(), m_world.mercy());
            m_debugConsole.render(*p, rect(), m_world, static_cast<float>(m_clock.fps()));
            m_dialogue.render(*p, rect(), m_totalTime);
        } else if (m_state == GameState::Paused) {
            m_pauseOverlay.render(*p, rect());
            drawPauseRecap(*p);
        }
        if (m_settings.subtitles && !m_dialogue.active()) drawSubtitles(*p, rect());
        m_overlay.end();
        m_overlay.render(this, targetFbo, pixelW, pixelH);
    }
    if (m_state != GameState::Playing) Input::instance().update();
}

void GLView::drawSubtitles(QPainter& p, const QRect& bounds) {
    auto& subs = SubtitleSystem::instance();
    if (!subs.hasActive()) return;
    float alpha = subs.alpha();
    if (alpha <= 0.01f) return;

    QString speaker = subs.currentSpeaker();
    QString full = speaker.isEmpty() ? subs.currentText() : QString("%1: %2").arg(speaker).arg(subs.currentText());
    int shown = std::min(static_cast<int>(full.size()), static_cast<int>(subs.elapsed() * 55.0f));   // typewriter reveal
    p.save();
    const QFont& f = cachedFont("Georgia", 20, false, true);
    p.setFont(f);
    QFontMetrics fm(f);
    int w = fm.horizontalAdvance(full) + 36;
    QRect bg(bounds.center().x() - w / 2, bounds.bottom() - 120, w, 38);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, static_cast<int>(180 * alpha)));
    p.drawRoundedRect(bg, 5, 5);
    QColor col = subs.currentColor();
    col.setAlpha(static_cast<int>(255 * alpha));
    p.setPen(col);
    p.drawText(bg.adjusted(18, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, full.left(shown));
    p.restore();
}

void GLView::keyPressEvent(QKeyEvent* event) {
    const int key = event->key();
    if (event->isAutoRepeat()) { event->accept(); return; }

    if (m_state == GameState::Story) {
        if (key == Qt::Key_Escape) setGameState(GameState::Menu);
        event->accept();
        return;
    }

    if (inCinematic()) {
        if (m_cinematic) {
            if (key == Qt::Key_Escape) { m_cinematic->skip(); }
            else if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Right || key == Qt::Key_Down || key == Qt::Key_E) m_cinematic->onAdvanceKey();
        }
        event->accept();
        return;
    }

    if (key == Qt::Key_Escape) {
        if (m_state == GameState::Playing) {
            if (m_journalOpen) m_journalOpen = false;
            else if (m_inventoryOpen) m_inventoryOpen = false;
            else setGameState(GameState::Paused);
        } else if (m_state == GameState::Paused) {
            setGameState(GameState::Playing);
        }
        event->accept();
        return;
    }

    if (m_state == GameState::Playing) {
        if (m_dialogue.active()) {
            if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_E) m_dialogue.advance();
            event->accept();
            return;
        }
        if (key == Qt::Key_Space && m_world.mercy().isActive()) { humTap(); event->accept(); return; }
        if (key == Qt::Key_Tab) { m_journalOpen = !m_journalOpen; m_world.audio().playUi(2); return; }
        if (key == Qt::Key_I) { m_inventoryOpen = !m_inventoryOpen; return; }
        if (key == Qt::Key_M) { m_hud.toggleMap(); return; }
        if (key >= Qt::Key_1 && key <= Qt::Key_6) { m_inventoryOverlay.selectSlot(key - Qt::Key_1); return; }
        if (key == Qt::Key_F3) { m_debugConsole.toggleDebugOverlay(); return; }
        if (key == Qt::Key_F5) { m_debugConsole.giveAllItems(m_world); return; }
        if (key == Qt::Key_F6) { m_debugConsole.teleportToObjective(m_world); return; }
        if (key == Qt::Key_F8) { m_debugConsole.summonGhosts(m_world); return; }
        if (key == Qt::Key_F7) { m_debugConsole.unlockAllDoors(m_world); return; }
        if (m_journalOpen) {
            if (key == Qt::Key_Up) { m_journalOverlay.prevNote(); return; }
            if (key == Qt::Key_Down) { m_journalOverlay.nextNote(); return; }
        }
    }

    Input::instance().onKeyPressed(key);
    event->accept();
}

void GLView::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) { event->accept(); return; }
    Input::instance().onKeyReleased(event->key());
    event->accept();
}

void GLView::mouseMoveEvent(QMouseEvent* event) {
    if (m_state == GameState::Menu) {
        m_menuOverlay.onMouseMove(event->pos());
        if (m_menuOverlay.hoveredIndex() != m_menuHover) {
            m_menuHover = m_menuOverlay.hoveredIndex();
            if (m_menuHover >= 0) m_world.audio().playUi(0);
        }
        return;
    }
    if (m_state == GameState::Paused) {
        m_pauseOverlay.onMouseMove(event->pos());
        if (m_pauseOverlay.hoveredIndex() != m_pauseHover) {
            m_pauseHover = m_pauseOverlay.hoveredIndex();
            if (m_pauseHover >= 0) m_world.audio().playUi(0);
        }
        return;
    }
    if (m_state == GameState::Story) {
        int before = m_storyBrowser.hoveredItem();
        m_storyBrowser.onMouseMove(event->pos());
        if (m_storyBrowser.hoveredItem() != before && m_storyBrowser.hoveredItem() >= 0) m_world.audio().playUi(0);
        return;
    }
    if (inCinematic() && m_cinematic) {
        m_cinematic->onMouseMove(event->pos(), rect());
        return;
    }
    if (!m_cursorCaptured) return;

    QPoint center(width() / 2, height() / 2);
    if (m_ignoreNextMove) {      // the synthetic move generated by re-centring the cursor
        m_ignoreNextMove = false;
        if (event->pos() == center) return;
    }
    int dx = event->pos().x() - center.x();
    int dy = event->pos().y() - center.y();
    if (dx != 0 || dy != 0) {
        Input::instance().onMouseMove(dx, dy);
        m_ignoreNextMove = true;
        QCursor::setPos(mapToGlobal(center));
    }
    event->accept();
}

void GLView::mousePressEvent(QMouseEvent* event) {
    if (m_state == GameState::Menu) {
        MenuAction act = m_menuOverlay.onMouseClick(event->pos());
        if (act != MenuAction::None) m_world.audio().playUi(1);
        if (act == MenuAction::NewGame) {
            startNewGame();
        } else if (act == MenuAction::Continue) {
            SaveData data;
            if (SaveGame::loadFromFile(savePath().toStdString(), data)) {
                makeCurrent();   // rebuilding the manor uploads GPU meshes
                m_world.applySave(data);
                doneCurrent();
                applySettings();
                m_hud.resetSession();
                m_dialogue.clear();
                setGameState(GameState::Playing);
            }
        } else if (act == MenuAction::Settings) {
            SettingsDialog dlg(m_settings, this);
            dlg.exec();
            applyQuality();
        } else if (act == MenuAction::Credits) {
            openStory();   // the "Credits" slot is now the story & endings overview
        } else if (act == MenuAction::Quit) {
            QApplication::quit();
        }
        return;
    }

    if (m_state == GameState::Story) {
        StoryAction act = m_storyBrowser.onClick(event->pos());
        if (act != StoryAction::None) m_world.audio().playUi(1);
        if (act == StoryAction::Back) setGameState(GameState::Menu);
        else if (act == StoryAction::WatchIntro) startStoryMovie(true, 0);
        else if (act == StoryAction::WatchEnding) startStoryMovie(false, m_storyBrowser.endingIndex());
        return;
    }

    if (inCinematic() && m_cinematic) {
        m_cinematic->onClick(event->pos(), rect());
        return;
    }

    if (m_state == GameState::Paused) {
        PauseAction act = m_pauseOverlay.onMouseClick(event->pos());
        if (act != PauseAction::None) m_world.audio().playUi(1);
        if (act == PauseAction::Resume) {
            setGameState(GameState::Playing);
        } else if (act == PauseAction::Settings) {
            SettingsDialog dlg(m_settings, this);
            dlg.exec();
            applyQuality();
        } else if (act == PauseAction::Save) {
            SaveGame::saveToFile(savePath().toStdString(), m_world.makeSave());
            SubtitleSystem::instance().show("Hollowmere", "Checkpoint saved.", 2.5f);
            setGameState(GameState::Playing);
        } else if (act == PauseAction::LoadLastCheckpoint) {
            SaveData data;
            if (SaveGame::loadFromFile(savePath().toStdString(), data)) {
                makeCurrent();
                m_world.applySave(data);
                doneCurrent();
                applySettings();
                m_dialogue.clear();
                SubtitleSystem::instance().show("Hollowmere", "Checkpoint loaded.", 2.5f);
            }
            setGameState(GameState::Playing);
        } else if (act == PauseAction::QuitToMenu) {
            setGameState(GameState::Menu);
        }
        return;
    }

    // Playing
    if (m_dialogue.active()) {
        m_dialogue.advance();
        event->accept();
        return;
    }
    if (!m_cursorCaptured) {
        setCursorCaptured(true);
        return;
    }
    if (m_world.mercy().isActive() && event->button() == Qt::LeftButton) {   // hum rhythm: click on the beat
        humTap();
        event->accept();
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
    if (m_state == GameState::Playing) setGameState(GameState::Paused);
    QOpenGLWidget::focusOutEvent(event);
}

} // namespace hm
