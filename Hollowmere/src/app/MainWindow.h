#pragma once

#include <QMainWindow>
#include "gfx/GLView.h"
#include "GameState.h"

namespace hm {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    GLView* m_glView = nullptr;
    GameState m_state = GameState::Playing;
};

} // namespace hm
