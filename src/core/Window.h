#pragma once

#include <string>

struct GLFWwindow;

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    bool        shouldClose() const;
    void        swapBuffers();
    void        pollEvents();
    void        setCursorDisabled(bool disabled);
    int         getWidth()  const { return m_width; }
    int         getHeight() const { return m_height; }
    float       getAspectRatio() const;
    GLFWwindow* getHandle() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
    int         m_width;
    int         m_height;
};
