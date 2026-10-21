#pragma once

#include <unordered_set>
#include <QPoint>

namespace hm {

class Input {
public:
    static Input& instance();

    void update(); // resets single-frame pressed/released buffers & mouse deltas

    void onKeyPressed(int key);
    void onKeyReleased(int key);

    void onMouseMove(int deltaX, int deltaY);
    void onMouseButtonPressed(int button);
    void onMouseButtonReleased(int button);
    void onFocusLost();

    bool isKeyDown(int key) const;
    bool wasKeyPressed(int key) const;
    bool wasKeyReleased(int key) const;

    bool isMouseButtonDown(int button) const;
    bool wasMouseButtonPressed(int button) const;

    int mouseDeltaX() const { return m_deltaX; }
    int mouseDeltaY() const { return m_deltaY; }

    bool isMouseLocked() const { return m_mouseLocked; }
    void setMouseLocked(bool locked) { m_mouseLocked = locked; }

private:
    Input() = default;

    std::unordered_set<int> m_keysDown;
    std::unordered_set<int> m_keysPressed;
    std::unordered_set<int> m_keysReleased;

    std::unordered_set<int> m_mouseButtonsDown;
    std::unordered_set<int> m_mouseButtonsPressed;

    int m_deltaX = 0;
    int m_deltaY = 0;
    bool m_mouseLocked = true;
};

} // namespace hm
