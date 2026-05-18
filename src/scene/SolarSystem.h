#pragma once

#include <vector>
#include <memory>
#include "Star.h"
#include "Planet.h"
#include "Orbit.h"

class Camera;
class Shader;

class SolarSystem {
public:
    SolarSystem();

    void update(const class Time& time);
    void drawAll(Camera& camera, float aspectRatio);

    CelestialBody* getSun()  { return m_star.get(); }
    const Star*    getStar() const { return m_star.get(); }

    void  setTimeScale(float scale);
    float getTimeScale() const { return m_timeScale; }

private:
    std::unique_ptr<Star>                m_star;
    std::vector<std::unique_ptr<Planet>> m_planets;
    std::vector<std::unique_ptr<Orbit>>  m_orbits;

    Shader m_planetShader;
    Shader m_sunShader;

    float m_timeScale = 1.0f;
};
