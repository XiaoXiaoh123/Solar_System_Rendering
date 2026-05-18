#pragma once

class Time {
public:
    Time();

    void tick();     // call once per frame
    void reset();

    float getDeltaTime()   const { return m_deltaTime * m_timeScale; }
    float getRawDeltaTime() const { return m_deltaTime; }
    float getElapsedTime() const { return m_elapsedTime; }
    float getTimeScale()   const { return m_timeScale; }

    void  setTimeScale(float scale);

    // GLFW time wrappers
    static float getGlobalTime();

private:
    float m_lastFrameTime  = 0.0f;
    float m_deltaTime      = 0.0f;
    float m_timeScale      = 10.0f;
    float m_elapsedTime    = 0.0f;
};
