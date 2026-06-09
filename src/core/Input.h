#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

class Input {
public:
    explicit Input(GLFWwindow* window);

    void update();
    void resetMouse();

    bool isKeyDown(int key) const;

    glm::vec2 getMouseDelta()    const { return m_mouseDelta; }
    glm::vec2 getMousePosition() const { return m_mousePos; }

private:
    GLFWwindow* m_window;

    glm::vec2 m_mousePos       = glm::vec2(0.0f);
    glm::vec2 m_lastMousePos   = glm::vec2(0.0f);
    glm::vec2 m_mouseDelta     = glm::vec2(0.0f);
    bool      m_firstMouse     = true;
};
