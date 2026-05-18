#include "Window.h"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "[GLFW Error " << error << "] " << description << std::endl;
}

static void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height)
{
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Track actual framebuffer size (may differ from window size on HiDPI)
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);

    // Resize callback keeps viewport in sync with window
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::setCursorDisabled(bool disabled) {
    glfwSetInputMode(m_window, GLFW_CURSOR,
        disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Window::updateFramebufferSize() {
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
}

float Window::getAspectRatio() const {
    return static_cast<float>(m_width) / static_cast<float>(m_height);
}
