#include "Input.h"

namespace hm {

Input& Input::instance() {
    static Input s_instance;
    return s_instance;
}

void Input::update() {
    m_keysPressed.clear();
    m_keysReleased.clear();
    m_mouseButtonsPressed.clear();
    m_deltaX = 0;
    m_deltaY = 0;
}

void Input::onKeyPressed(int key) {
    if (m_keysDown.find(key) == m_keysDown.end()) {
        m_keysPressed.insert(key);
    }
    m_keysDown.insert(key);
}

void Input::onKeyReleased(int key) {
    m_keysDown.erase(key);
    m_keysReleased.insert(key);
}

void Input::onMouseMove(int deltaX, int deltaY) {
    m_deltaX += deltaX;
    m_deltaY += deltaY;
}

void Input::onMouseButtonPressed(int button) {
    if (m_mouseButtonsDown.find(button) == m_mouseButtonsDown.end()) {
        m_mouseButtonsPressed.insert(button);
    }
    m_mouseButtonsDown.insert(button);
}

void Input::onMouseButtonReleased(int button) {
    m_mouseButtonsDown.erase(button);
}

void Input::onFocusLost() {
    m_keysDown.clear();
    m_keysPressed.clear();
    m_keysReleased.clear();
    m_mouseButtonsDown.clear();
    m_mouseButtonsPressed.clear();
    m_deltaX = 0;
    m_deltaY = 0;
}

bool Input::isKeyDown(int key) const {
    return m_keysDown.find(key) != m_keysDown.end();
}

bool Input::wasKeyPressed(int key) const {
    return m_keysPressed.find(key) != m_keysPressed.end();
}

bool Input::wasKeyReleased(int key) const {
    return m_keysReleased.find(key) != m_keysReleased.end();
}

bool Input::isMouseButtonDown(int button) const {
    return m_mouseButtonsDown.find(button) != m_mouseButtonsDown.end();
}

bool Input::wasMouseButtonPressed(int button) const {
    return m_mouseButtonsPressed.find(button) != m_mouseButtonsPressed.end();
}

} // namespace hm
