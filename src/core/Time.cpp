#include "Time.h"
#include <GLFW/glfw3.h>
#include <algorithm>

Time::Time() {
    m_lastFrameTime = static_cast<float>(glfwGetTime());
}

void Time::tick() {
    float currentTime = static_cast<float>(glfwGetTime());
    m_deltaTime       = currentTime - m_lastFrameTime;
    m_lastFrameTime   = currentTime;
    m_elapsedTime    += m_deltaTime * m_timeScale;
}

void Time::reset() {
    m_lastFrameTime = static_cast<float>(glfwGetTime());
    m_elapsedTime   = 0.0f;
}

void Time::setTimeScale(float scale) {
    m_timeScale = std::clamp(scale, 0.0f, 1000.0f);
}

float Time::getGlobalTime() {
    return static_cast<float>(glfwGetTime());
}
