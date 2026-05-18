#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

class Input {
public:
    Input(GLFWwindow* window);

    void update();          // call at start of each frame
    void endFrame();        // call at end of each frame

    // Keyboard
    bool isKeyDown(int key) const;
    bool isKeyPressed(int key) const;   // true only on first frame of press
    bool isKeyReleased(int key) const;

    // Mouse
    glm::vec2 getMouseDelta()    const { return m_mouseDelta; }
    glm::vec2 getMousePosition() const { return m_mousePos; }
    float     getScrollDelta()   const { return m_scrollDelta; }

private:
    GLFWwindow* m_window;

    glm::vec2 m_mousePos       = glm::vec2(0.0f);
    glm::vec2 m_lastMousePos   = glm::vec2(0.0f);
    glm::vec2 m_mouseDelta     = glm::vec2(0.0f);
    float     m_scrollDelta    = 0.0f;
    bool      m_firstMouse     = true;

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};
