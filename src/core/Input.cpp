#include "Input.h"
#include <GLFW/glfw3.h>

Input::Input(GLFWwindow* window) : m_window(window) {}

void Input::update() {
    m_mouseDelta = glm::vec2(0.0f);

    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    m_mousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));

    if (m_firstMouse) {
        m_lastMousePos = m_mousePos;
        m_firstMouse   = false;
    }

    m_mouseDelta   = m_mousePos - m_lastMousePos;
    m_lastMousePos = m_mousePos;
}

bool Input::isKeyDown(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}
