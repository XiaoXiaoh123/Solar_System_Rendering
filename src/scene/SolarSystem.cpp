#include "SolarSystem.h"
#include "../core/Camera.h"
#include "../core/Time.h"
#include "../utils/Constants.h"
#include "../render/Shader.h"
#include <glad/gl.h>

SolarSystem::SolarSystem() {
    m_planetShader.load("assets/shaders/planet.vert", "assets/shaders/planet.frag");
    m_sunShader.load("assets/shaders/sun.vert", "assets/shaders/sun.frag");

    // --- Sun ---
    {
        CelestialParams p;
        p.name        = "Sun";
        p.radius      = Constants::SUN_RADIUS;
        p.orbitRadius = 0.0f;
        m_star = std::make_unique<Star>(p);
    }

    // --- Planets ---
    auto addPlanet = [&](const CelestialParams& p) {
        auto planet = std::make_unique<Planet>(p);
        auto orbit  = std::make_unique<Orbit>(p.orbitRadius);
        m_planets.push_back(std::move(planet));
        m_orbits.push_back(std::move(orbit));
    };

    addPlanet({"Mercury",  Constants::MERCURY_RADIUS,  Constants::MERCURY_ORBIT,  Constants::MERCURY_ORBIT_SPEED,  0.017f,   0.03f});
    addPlanet({"Venus",    Constants::VENUS_RADIUS,    Constants::VENUS_ORBIT,    Constants::VENUS_ORBIT_SPEED,    0.004f,   2.64f});
    addPlanet({"Earth",    Constants::EARTH_RADIUS,    Constants::EARTH_ORBIT,    Constants::EARTH_ORBIT_SPEED,    1.0f,     23.44f});
    addPlanet({"Mars",     Constants::MARS_RADIUS,     Constants::MARS_ORBIT,     Constants::MARS_ORBIT_SPEED,     0.89f,    25.19f});
    addPlanet({"Jupiter",  Constants::JUPITER_RADIUS,  Constants::JUPITER_ORBIT,  Constants::JUPITER_ORBIT_SPEED,  2.45f,    3.13f});
    addPlanet({"Saturn",   Constants::SATURN_RADIUS,   Constants::SATURN_ORBIT,   Constants::SATURN_ORBIT_SPEED,   2.0f,     26.73f});
    addPlanet({"Uranus",   Constants::URANUS_RADIUS,   Constants::URANUS_ORBIT,   Constants::URANUS_ORBIT_SPEED,   1.4f,     97.77f});
    addPlanet({"Neptune",  Constants::NEPTUNE_RADIUS,  Constants::NEPTUNE_ORBIT,  Constants::NEPTUNE_ORBIT_SPEED,  1.5f,     28.32f});
}

void SolarSystem::update(const Time& time) {
    m_star->update(time);
    for (auto& planet : m_planets) {
        planet->update(time);
    }
}

void SolarSystem::drawAll(Camera& camera, float aspectRatio) {
    glm::mat4 view       = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);

    // Draw orbit rings
    for (auto& orbit : m_orbits) {
        orbit->draw(view, projection);
    }

    // Draw sun
    m_sunShader.use();
    m_sunShader.setMat4("uView", view);
    m_sunShader.setMat4("uProjection", projection);
    m_sunShader.setVec3("uViewPos", camera.getPosition());
    m_star->draw(m_sunShader);

    // Draw planets
    m_planetShader.use();
    m_planetShader.setMat4("uView", view);
    m_planetShader.setMat4("uProjection", projection);
    m_planetShader.setVec3("uViewPos", camera.getPosition());
    m_planetShader.setVec3("uLightPos", glm::vec3(0.0f));
    m_planetShader.setVec3("uLightColor", glm::vec3(1.0f, 0.95f, 0.8f));
    m_planetShader.setFloat("uAmbientStrength", 0.08f);

    for (auto& planet : m_planets) {
        planet->draw(m_planetShader);
    }
}

void SolarSystem::setTimeScale(float scale) {
    m_timeScale = scale;
}
