#include "SolarSystem.h"
#include "../core/Camera.h"
#include "../core/Time.h"
#include "../utils/Constants.h"
#include "../render/Shader.h"
#include <glad/gl.h>

SolarSystem::SolarSystem() {
    m_planetShader.load("assets/shaders/planet.vert", "assets/shaders/planet.frag");
    m_sunShader.load("assets/shaders/sun.vert", "assets/shaders/sun.frag");
    m_atmosphereShader.load("assets/shaders/atmosphere.vert", "assets/shaders/atmosphere.frag");

    // --- Sun ---
    {
        CelestialParams p;
        p.name        = "Sun";
        p.radius      = Constants::SUN_RADIUS;
        p.orbitRadius = 0.0f;
        p.texturePath = "assets/textures/sun.jpg";
        m_star = std::make_unique<Star>(p);
    }

    // --- Planets (real orbital & rotation data) ---
    auto addPlanet = [&](const CelestialParams& p) {
        auto planet = std::make_unique<Planet>(p);
        auto orbit  = std::make_unique<Orbit>(p.orbitRadius);
        m_planets.push_back(std::move(planet));
        m_orbits.push_back(std::move(orbit));
    };

    addPlanet({"Mercury",  Constants::MERCURY_RADIUS,  Constants::MERCURY_ORBIT,
               Constants::MERCURY_ORBIT_PERIOD, Constants::MERCURY_ROT_PERIOD, Constants::MERCURY_TILT,
                "assets/textures/mercury.jpg"});
    addPlanet({"Venus",    Constants::VENUS_RADIUS,    Constants::VENUS_ORBIT,
               Constants::VENUS_ORBIT_PERIOD,   Constants::VENUS_ROT_PERIOD,   Constants::VENUS_TILT,
                "assets/textures/venus_surface.jpg"});
    addPlanet({"Earth",    Constants::EARTH_RADIUS,    Constants::EARTH_ORBIT,
               Constants::EARTH_ORBIT_PERIOD,   Constants::EARTH_ROT_PERIOD,   Constants::EARTH_TILT,
               "assets/textures/earth_daymap.jpg",
               true, 1.08f}); // hasAtmosphere, atmosphereScale
    addPlanet({"Mars",     Constants::MARS_RADIUS,     Constants::MARS_ORBIT,
               Constants::MARS_ORBIT_PERIOD,    Constants::MARS_ROT_PERIOD,    Constants::MARS_TILT,
                "assets/textures/mars.jpg"});
    addPlanet({"Jupiter",  Constants::JUPITER_RADIUS,  Constants::JUPITER_ORBIT,
               Constants::JUPITER_ORBIT_PERIOD, Constants::JUPITER_ROT_PERIOD, Constants::JUPITER_TILT,
                "assets/textures/jupiter.jpg"});
    addPlanet({"Saturn",   Constants::SATURN_RADIUS,   Constants::SATURN_ORBIT,
               Constants::SATURN_ORBIT_PERIOD,  Constants::SATURN_ROT_PERIOD,  Constants::SATURN_TILT,
                "assets/textures/saturn.jpg"});
    addPlanet({"Uranus",   Constants::URANUS_RADIUS,   Constants::URANUS_ORBIT,
               Constants::URANUS_ORBIT_PERIOD,  Constants::URANUS_ROT_PERIOD,  Constants::URANUS_TILT,
                "assets/textures/uranus.jpg"});
    addPlanet({"Neptune",  Constants::NEPTUNE_RADIUS,  Constants::NEPTUNE_ORBIT,
               Constants::NEPTUNE_ORBIT_PERIOD, Constants::NEPTUNE_ROT_PERIOD, Constants::NEPTUNE_TILT,
                "assets/textures/neptune.jpg"});

    // Find Earth pointer for Moon parent
    for (auto& p : m_planets) {
        if (p->getName() == "Earth") {
            m_earth = p.get();
            break;
        }
    }

    // --- Moon ---
    {
        CelestialParams m;
        m.name               = "Moon";
        m.radius             = Constants::MOON_RADIUS;
        m.orbitRadius        = Constants::MOON_ORBIT;
        m.orbitPeriodDays    = Constants::MOON_ORBIT_PERIOD;
        m.rotationPeriodHours = Constants::MOON_ROT_PERIOD;
        m.texturePath        = "assets/textures/moon.jpg";
        m_moon = std::make_unique<Planet>(m);
        m_moon->setParent(m_earth);

        // Moon orbit (drawn around Earth's position, not origin)
        m_moonOrbit = std::make_unique<Orbit>(Constants::MOON_ORBIT,
                                              glm::vec3(0.4f, 0.4f, 0.5f));
    }
}

void SolarSystem::update(const Time& time) {
    m_star->update(time);
    for (auto& planet : m_planets) {
        planet->update(time);
    }
    m_moon->update(time);
}

void SolarSystem::drawAll(Camera& camera, float aspectRatio) {
    glm::mat4 view       = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);

    for (auto& orbit : m_orbits) {
        orbit->draw(view, projection);
    }
    // Moon orbit centered on Earth
    m_moonOrbit->draw(view, projection, m_earth->getWorldPosition());

    m_sunShader.use();
    m_sunShader.setMat4("uView", view);
    m_sunShader.setMat4("uProjection", projection);
    m_star->draw(m_sunShader);

    m_planetShader.use();
    m_planetShader.setMat4("uView", view);
    m_planetShader.setMat4("uProjection", projection);
    m_planetShader.setVec3("uLightPos", glm::vec3(0.0f));
    m_planetShader.setVec3("uLightColor", glm::vec3(1.0f, 0.95f, 0.8f));
    m_planetShader.setInt("uDebugMode", 0);  // 0=lit, 1=unlit diagnostic
    m_planetShader.setFloat("uAmbientStrength", m_ambientStrength);

    for (auto& planet : m_planets) {
        planet->draw(m_planetShader);
    }
    m_moon->draw(m_planetShader);

    // --- Atmosphere pass (transparent overlay) ---
    if (m_showAtmosphere) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // don't write depth for transparent atmosphere

        m_atmosphereShader.use();
        m_atmosphereShader.setMat4("uView", view);
        m_atmosphereShader.setMat4("uProjection", projection);
        m_atmosphereShader.setVec3("uViewPos", camera.getPosition());
        m_atmosphereShader.setVec3("uLightPos", glm::vec3(0.0f));
        // Earth-like atmosphere parameters
        m_atmosphereShader.setVec3("uRayleighColor", glm::vec3(0.3f, 0.6f, 1.0f));
        m_atmosphereShader.setFloat("uRayleighScaleHeight", 0.15f);
        m_atmosphereShader.setFloat("uMieScaleHeight", 0.05f);

        for (auto& planet : m_planets) {
            planet->drawAtmosphere(m_atmosphereShader);
        }
        m_moon->drawAtmosphere(m_atmosphereShader);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

void SolarSystem::setTimeScale(float scale) {
    m_timeScale = scale;
}
