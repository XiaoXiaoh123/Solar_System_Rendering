#include "Input.h"
#include <GLFW/glfw3.h>

Input::Input(GLFWwindow* window) : m_window(window) {
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetWindowUserPointer(window, this);
}

void Input::update() {
    m_mouseDelta  = glm::vec2(0.0f);
    m_scrollDelta = 0.0f;

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

void Input::endFrame() {
    // scroll delta already reset each frame in update()
}

bool Input::isKeyDown(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS; // GLFW doesn't track "just pressed"
}

bool Input::isKeyReleased(int key) const {
    return glfwGetKey(m_window, key) == GLFW_RELEASE;
}

void Input::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    // handled in update() via glfwGetCursorPos
}

void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        input->m_scrollDelta = static_cast<float>(yoffset);
    }
}
